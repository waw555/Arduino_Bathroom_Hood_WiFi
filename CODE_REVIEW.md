# Code Review Notes (ESP8266 Bathroom Hood)

## Critical issues

1. `LittleFS.begin()` is called **after** creating and starting `GyverPortal` with `ui(&LittleFS)` and `ui.start(...)`.
   The filesystem should be mounted before portal operations that may need FS.
2. In `setup()`, the code blocks forever in `while (WiFi.status() != WL_CONNECTED)` even after AP mode is started. If STA never connects, the main loop logic never runs.
3. DHT readings are assigned directly to `int` without validating `NaN`; failed sensor reads will produce invalid values and can flip relay logic unpredictably.

## Reliability & correctness improvements

- Add a finite connection strategy: timeout, then run AP+portal and continue normal `loop()` processing.
- Validate and clamp settings (`tempOn/off`, `humiOn/off`, hysteresis) before using in control logic.
- Avoid duplicated relay `digitalWrite` branches by extracting a helper (`setFan(bool on)`), reducing branch errors.
- Persist fewer globals duplicated from `data` (`operatingMode`, `invertOut`, etc.) to avoid divergence between RAM state and stored settings.

## Performance/maintainability

- Relay write logic is repeated many times in `loop()`; refactor to helper for code size and readability.
- UI markup for `/connect` and AP `/connect` is duplicated; extract builder fragment function to avoid drift.
- `delay(2000)` in `setup()` and long blocking loops increase boot latency and reduce responsiveness.

