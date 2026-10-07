#include "network_service.h"
#include "system_service.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <cstring>
#include <cmath>

namespace {
    constexpr uint8_t  SSID_MAX_LEN         = 32;
    constexpr uint8_t  PASS_MAX_LEN         = 64;
    constexpr uint8_t  FORECAST_HOURS       = 5;
    constexpr uint32_t CONNECT_TIMEOUT_MS   = 12000;
    constexpr uint32_t PORTAL_STOP_DELAY_MS = 100;

    WebServer g_server(80);
    DNSServer g_dns_server;
    Preferences g_prefs;
    bool g_portal_submitted = false;

    static const char PAGE_HEADER[] PROGMEM = R"rawliteral(
        <!DOCTYPE html><html><head>
        <meta name='viewport' content='width=device-width,initial-scale=1'>
        <style>
        body {background: #2C2C2C;color: #F2F2ED;font-family: sans-serif;padding: 1em;max-width: 600px;margin: 0 auto;}
        h2 {text-align: center;}
        ul {list-style: none;padding: 0;}
        li {margin: 0.5em 0;}
        a {display: block;padding: 1em;background: #118AB2;border-radius: 8px;text-decoration: none;color: #fff;font-size: 1.2em;text-align: center;}
        a:hover {background: #66A3BF;color: #2C2C2C;}.form {display: none;margin-top: 1em;border: 1px solid #ddd;padding: 1em;border-radius: 8px;}
        input,button {width: 100%;padding: 1em;font-size: 1.1em;margin-top: 1em;box-sizing: border-box;}
        button {background: #2A7C13;border: none;color: #fff;border-radius: 4px;}
        </style></head><body><h2>Select Wi-Fi Network</h2><ul>
        )rawliteral";

    static const char PAGE_FOOTER[] PROGMEM = R"rawliteral(</ul>
        <div id=form class=form>
        <h2 id=title>Enter Password</h2>
        <h2 id="selected_ssid"></h2>
        <form method='POST' action='/connect'>
        <input type=hidden name=ssid id=ssid>
        <input type=password name=password placeholder='Password' autofocus>
        <button>Connect</button>
        </form>
        </div>
        <script>
        function sel(s){
            document.getElementById('ssid').value = s;
            document.getElementById("title").textContent = "Enter password for:";
            document.getElementById("selected_ssid").textContent = s;
            document.getElementById('form').style.display = 'block';
        }
        </script></body></html>
        )rawliteral";

    void render_portal_page() {
        g_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
        g_server.send(200, "text/html", "");
        g_server.sendContent_P(PAGE_HEADER);

        int16_t found = WiFi.scanComplete();
        if (found <= 0) {
            g_server.sendContent("<li>No networks found</li>");
        } else {
            char buf[128];
            for (int16_t i = 0; i < found && i < 8; ++i) {
                snprintf(buf, sizeof(buf), "<li><a href='#' onclick=\"sel(this.innerText)\">%s</a></li>", WiFi.SSID(i).c_str());
                g_server.sendContent(buf);
            }
        }
        g_server.sendContent_P(PAGE_FOOTER);
        g_server.sendContent("");
    }

    void handle_portal_save() {
        if (!g_server.hasArg("ssid") || !g_server.hasArg("password")) {
            g_server.send(400, "text/plain", "Missing credentials");
            return;
        }

        char ssid[SSID_MAX_LEN + 1] = {0};
        char pass[PASS_MAX_LEN + 1] = {0};
        strncpy(ssid, g_server.arg("ssid").c_str(), sizeof(ssid) - 1);
        strncpy(pass, g_server.arg("password").c_str(), sizeof(pass) - 1);

        g_prefs.begin("wifi", false);
        g_prefs.putString("ssid", ssid);
        g_prefs.putString("pass", pass);
        g_prefs.end();

        char resp[128];
        snprintf(resp, sizeof(resp), "<h3>Connecting to %s...</h3><p>Window will close.</p>", ssid);
        g_server.send(200, "text/html", resp);

        g_portal_submitted = true;
    }
}


void NetworkService::init(AppState& state) {
    load_stored_location(state.weather.location);
}

bool NetworkService::start_portal(AppState& state, const char* ap_ssid) {
    if (!state.is_charging && !(state.battery >= BatteryState::TWO_BAR)) {
        return false;
    }

    WiFi.mode(WIFI_AP_STA);
    WiFi.scanNetworks(true);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);

    WiFi.softAP(ap_ssid);
    g_dns_server.start(53, "*", WiFi.softAPIP());

    g_server.on("/", render_portal_page);
    g_server.on("/connect", HTTP_POST, handle_portal_save);
    g_server.onNotFound([]() {
        g_server.sendHeader("Location", "/", true);
        g_server.send(302, "text/plain", "");
    });

    g_server.begin();
    g_portal_submitted = false;
    portal_stop_pending_ = false;
    state.net_state = NetworkState::PORTAL_ACTIVE;
    return true;
}

void NetworkService::stop_portal(AppState& state) {
    if (state.net_state != NetworkState::PORTAL_ACTIVE) return;

    g_dns_server.stop();
    g_server.stop();
    WiFi.scanDelete();
    WiFi.softAPdisconnect(true);

    portal_stop_pending_ = false;
    g_portal_submitted = false;
    state.net_state = NetworkState::IDLE;
}

NetworkState NetworkService::poll(AppState& state) {
    if (state.net_state == NetworkState::PORTAL_ACTIVE) {
        g_dns_server.processNextRequest();
        g_server.handleClient();

        if (g_portal_submitted) {
            g_portal_submitted = false;
            portal_stop_pending_ = true;
            portal_transition_ms_ = millis();
        }

        if (portal_stop_pending_ && (millis() - portal_transition_ms_ >= PORTAL_STOP_DELAY_MS)) {
            stop_portal(state);
            connect_stored(state);
        }
        return NetworkState::IDLE;
    }

    if (WiFi.status() == WL_CONNECTED) {
        state.net_state = NetworkState::CONNECTED;

        SystemService::sync_time_ntp();

        if (!state.weather.location.is_valid) {
            fetch_ip_location(state.weather.location);
        }

        state.weather.is_valid = state.weather.location.is_valid && fetch_weather_forecast(state.weather);
        disconnect(state);
        Serial.printf("WIFI DISCONNECTED\n");

        return NetworkState::IDLE;
    }
    return NetworkState::IDLE;
}

bool NetworkService::connect_stored(AppState& state) {
    if (state.net_state == NetworkState::PORTAL_ACTIVE || (!state.is_charging && state.battery < BatteryState::TWO_BAR)) {
        return false;
    }

    char ssid[SSID_MAX_LEN + 1] = {0};
    char pass[PASS_MAX_LEN + 1] = {0};

    g_prefs.begin("wifi", true);
    size_t ssid_len = g_prefs.getString("ssid", ssid, sizeof(ssid));
    g_prefs.getString("pass", pass, sizeof(pass));
    g_prefs.end();

    if (ssid_len == 0) {
        cached_ssid_[0] = '\0';
        state.net_state = NetworkState::IDLE;
        return false;
    }

    strncpy(cached_ssid_, ssid, sizeof(cached_ssid_) - 1);
    cached_ssid_[sizeof(cached_ssid_) - 1] = '\0';

    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_11dBm);
    WiFi.begin(ssid, pass);

    connect_start_ms_ = millis();
    state.net_state = NetworkState::CONNECTING;
    return true;
}

void NetworkService::disconnect(AppState& state) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    state.net_state = NetworkState::IDLE;
}

void NetworkService::load_stored_location(LocationData& loc) {
    g_prefs.begin("geo", true);
    loc.latitude  = g_prefs.getFloat("lat", 0.0f);
    loc.longitude = g_prefs.getFloat("lon", 0.0f);
    size_t len    = g_prefs.getString("city", loc.city, sizeof(loc.city));
    loc.city[sizeof(loc.city) - 1] = '\0';
    g_prefs.end();

    loc.is_valid = (loc.latitude != 0.0f && loc.longitude != 0.0f && len > 0);
}

bool NetworkService::fetch_ip_location(LocationData& loc) {
    HTTPClient http;
    http.begin("http://ip-api.com/json/?fields=status,city,lat,lon");
    const int code = http.GET();

    bool success = false;
    if (code == HTTP_CODE_OK) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, http.getString());
        if (!err && doc["status"] == "success") {
            loc.latitude  = doc["lat"];
            loc.longitude = doc["lon"];
            strncpy(loc.city, doc["city"] | "", sizeof(loc.city) - 1);
            loc.city[sizeof(loc.city) - 1] = '\0';
            loc.is_valid  = true;

            g_prefs.begin("geo", false);
            g_prefs.putFloat("lat", loc.latitude);
            g_prefs.putFloat("lon", loc.longitude);
            g_prefs.putString("city", loc.city);
            g_prefs.end();

            success = true;
        }
    }

    http.end();
    return success;
}

bool NetworkService::fetch_weather_forecast(WeatherData& weather) {
    char url[192];
    snprintf(url, sizeof(url),
             "http://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
             "&hourly=temperature_2m,wind_speed_10m,weather_code,is_day"
             "&timezone=auto&forecast_hours=6",
             weather.location.latitude,
             weather.location.longitude);
    
    HTTPClient http;
    http.begin(url);
    const int code = http.GET();

    bool success = false;
    if (code == HTTP_CODE_OK) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, http.getString());

        if (!err) {
            JsonArrayConst times  = doc["hourly"]["time"];
            JsonArrayConst temps  = doc["hourly"]["temperature_2m"];
            JsonArrayConst winds  = doc["hourly"]["wind_speed_10m"];
            JsonArrayConst codes  = doc["hourly"]["weather_code"];
            JsonArrayConst is_day = doc["hourly"]["is_day"];

            const uint8_t available = static_cast<uint8_t>(times.size() < FORECAST_HOURS ? times.size() : FORECAST_HOURS);

            for (uint8_t i = 0; i < available; ++i) {
                const char* time_str = times[i];
                uint8_t parsed_hour = 0;
                if (time_str != nullptr) {
                    const char* t_ptr = strchr(time_str, 'T');
                    if (t_ptr != nullptr && strlen(t_ptr) >= 3) {
                        parsed_hour = static_cast<uint8_t>(atoi(t_ptr + 1));
                    }
                }

                weather.forecast[i].hour          = parsed_hour;
                weather.forecast[i].temp_c        = static_cast<int8_t>(round(temps[i].as<float>()));
                weather.forecast[i].windspeed_kmh = static_cast<uint8_t>(round(winds[i].as<float>()));
                weather.forecast[i].code          = codes[i].as<uint8_t>();
                weather.forecast[i].is_day        = (is_day[i].as<int>() == 1);
            }

            weather.is_valid = (available > 0);
            success = weather.is_valid;
        }
    }

    http.end();
    if (!success) {
        weather.is_valid = false;
    }
    return success;
}