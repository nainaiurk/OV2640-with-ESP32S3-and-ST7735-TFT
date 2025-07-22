# Fixed: Automatic Timeout Removal

## Issue Fixed
The camera was automatically returning to live view after 2 seconds instead of waiting for the user to press the UP button again.

## Root Cause
The code had an automatic timeout condition:
```cpp
if ((up_button_current && !up_button_was_pressed) || (millis() - capture_show_start > CAPTURE_DISPLAY_TIME)) {
```

This `|| (millis() - capture_show_start > CAPTURE_DISPLAY_TIME)` part was causing the automatic return to live view after 2000ms (2 seconds).

## Solution Applied

### 1. Removed Automatic Timeout
**Before:**
```cpp
// Check for UP button to continue capturing or auto-return after timeout
if ((up_button_current && !up_button_was_pressed) || (millis() - capture_show_start > CAPTURE_DISPLAY_TIME)) {
    if (up_button_current && !up_button_was_pressed) {
        Serial.println("UP button pressed - resuming camera feed for next capture...");
    } else {
        Serial.println("Auto-timeout - resuming camera feed...");
    }
    // Return to live view
}
```

**After:**
```cpp
// Check ONLY for UP button to continue capturing (NO automatic timeout)
if (up_button_current && !up_button_was_pressed) {
    Serial.println("UP button pressed - resuming camera feed for next capture...");
    // Return to live view only when UP button is pressed
}
```

### 2. Cleaned Up Unused Code
- Removed `#define CAPTURE_DISPLAY_TIME 2000`
- Removed `static unsigned long capture_show_start = 0;`
- Removed `capture_show_start = millis();` assignment

## New Behavior

### ✅ **Current Behavior**
1. **Capture**: Press UP button → Image captured and displayed
2. **Display**: Captured image shows indefinitely with white border
3. **Continue**: Press UP button again → Returns to live camera feed
4. **Exit**: Press BACK button → Returns to menu

### ✅ **Key Improvements**
- **No Automatic Timeout**: Image stays displayed until user action
- **Manual Control**: User has full control over when to continue
- **Cleaner Code**: Removed unnecessary timeout logic
- **Better UX**: User can examine captured image for as long as needed

## Visual Flow
```
Live Camera Feed
       ↓ (UP button)
Capture Image
       ↓
Display Captured Image ← Stays here indefinitely
       ↓ (UP button again)
Live Camera Feed (ready for next capture)
```

## Build Status
- **Compilation**: ✅ Success
- **Memory**: Slightly reduced (removed unused variables)
- **Functionality**: Captured image now stays displayed until manual UP button press

The captured image will now remain visible indefinitely until you manually press the UP button to continue capturing more images!
