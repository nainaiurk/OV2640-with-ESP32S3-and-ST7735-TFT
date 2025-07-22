# Code Organization Documentation

## Overview
The ESP32-S3 Camera Gaming System has been refactored into a clean, modular architecture that separates concerns and keeps `main.cpp` minimal and organized.

## File Structure

### Core System Files

#### `main.cpp` - Application Entry Point
- **Purpose**: Minimal entry point containing only `setup()` and `loop()`
- **Size**: ~10 lines
- **Dependencies**: Only includes `app_controller.h`
- **Responsibilities**: 
  - Call `app_init()` in setup
  - Call `app_handle_mode()` in loop

#### `app_controller.h/cpp` - Main Application Controller
- **Purpose**: Central coordinator for all application modes and initialization
- **Key Functions**:
  - `app_init()` - Complete system initialization
  - `app_handle_mode()` - Main mode dispatcher
  - `handle_menu_mode()` - Menu navigation logic
  - `handle_object_detection_mode()` - AI/Object detection mode
  - `handle_game_menu_mode()` - Game selection logic
- **Dependencies**: Coordinates between all subsystems

### Display & UI System

#### `display.h/cpp` - Display Hardware Control
- **Purpose**: ST7735 TFT display initialization and basic drawing functions
- **Responsibilities**: Hardware-level display operations

#### `menu.h/cpp` - Menu System
- **Purpose**: Menu navigation, UI state management, and mode switching
- **Key Functions**:
  - `menu_init()` - Initialize menu system
  - `display_menu()` - Show main menu
  - `handle_menu_input()` - Process menu navigation
- **Dependencies**: Uses button.h for input handling

#### `button.h/cpp` - Button Input System
- **Purpose**: Button hardware control, debouncing, and input detection
- **Key Functions**:
  - `button_init()` - Initialize button pins with pullup resistors
  - `button_pressed()` - Debounced button press detection with release wait
- **Pin Configuration**: IO4=UP, IO5=DOWN, IO14=SELECT, IO13=BACK
- **Features**: Hardware debouncing, blocking input detection

### Camera System

#### `camera.h/cpp` - Camera Hardware Control
- **Purpose**: OV2640 camera initialization, configuration, and frame capture
- **Key Functions**:
  - `camera_init()` - Hardware initialization
  - `camera_configure()` - Set resolution, quality, etc.
  - `camera_get_frame()` - Capture frame buffer

#### `camera_mode.h/cpp` - Camera Display Mode
- **Purpose**: Live camera preview functionality
- **Key Functions**:
  - `handle_camera_mode()` - Main camera loop with FPS monitoring
  - `show_camera_error()` - Error handling for camera failures
- **Features**: Frame rate monitoring, error recovery, JPEG decoding

### Gaming System

#### `game_controller.h/cpp` - Game Mode Coordinator
- **Purpose**: Coordinate between different game modes and handle game-specific timing
- **Key Functions**:
  - `handle_game_modes()` - Game mode dispatcher
  - `run_snake_mode()`, `run_pong_mode()`, etc. - Individual game runners with timing

#### `games/` Folder - Individual Game Implementations
- **Structure**:
  - `games.h` - Common game definitions and function declarations
  - `snake.cpp` - Snake game implementation
  - `pong.cpp` - Pong game implementation
  - `breakout.cpp` - Breakout game implementation
  - `flappy_bird.cpp` - Flappy Bird game implementation
  - `memory.cpp` - Memory matching game implementation

## Key Benefits of This Organization

### 1. **Separation of Concerns**
- Each file has a single, well-defined responsibility
- Hardware control separated from application logic
- Game logic isolated from system management

### 2. **Maintainability**
- Easy to locate and modify specific functionality
- Clear dependency hierarchy
- Minimal coupling between modules

### 3. **Scalability**
- Easy to add new games (just add to `games/` folder)
- Easy to add new display modes
- Modular structure supports future enhancements

### 4. **Clean main.cpp**
- Only 10 lines of code
- No business logic mixed with system entry point
- Easy to understand application flow

### 5. **Testability**
- Each module can be tested independently
- Clear interfaces between components
- Hardware abstraction allows for easier testing

## Code Flow

```
main.cpp (setup/loop)
    ↓
app_controller.cpp (mode coordination)
    ↓
┌─── camera_mode.cpp (camera display)
├─── menu.cpp (navigation) ─── button.cpp (input handling)
├─── game_controller.cpp (game coordination)
│    └─── games/*.cpp (individual games) ─── button.cpp (input handling)
└─── display.cpp (hardware control)
```

## Adding New Features

### Adding a New Game
1. Create `games/new_game.cpp` with `init_new_game()` and `update_new_game()`
2. Add game enum to `menu.h`
3. Add case to game controller switch statement
4. Add menu entry to `menu.cpp`

### Adding a New Display Mode
1. Create `new_mode.h/cpp` files
2. Add mode enum to `menu.h`
3. Add handler to `app_controller.cpp`
4. Add menu entry if needed

### Modifying Hardware
1. Camera changes go in `camera.h/cpp`
2. Display changes go in `display.h/cpp`
3. Button changes go in `menu.h/cpp`

## Memory Usage
- **RAM**: 8.3% (27,308 bytes used)
- **Flash**: 10.6% (353,393 bytes used)
- Excellent memory efficiency maintained after refactoring
