## v3.1.0
 ### Overview
 - HARP 3.1.0 introduces significantly refactored code, major features, and many small improvements.

 ### Significant Refactoring
 - **Decoupling & Organization**:
   - The entire codebase has been decoupled and reorganized for improved modularity and readability.
 - **Token Management**:
   - Tokens are now managed through a shared resource accessible through a single settings tab.
 - **Logging & Error Reporting**:
   - Utilities for logging, error reporting, and more have been broadened and improved.

 ### Major Features
 - **Welcome / Walkthrough**:
   - Upon startup a Welcome / Walkthrough dialog appears to provide an overview of HARP and assist new users.
 - **Model Tags for Loading Failures**:
   - Upon loading failures, model paths are now added to dropdown with an appropriate tag.

 ### Other Improvements
 - **Multiple Window Interactions**:
   - Tracks are now added to current window instead of main window when "Current Window" is selected.
 - **Status Panel Visibility**:
   - TODO - toggleable view and scrollability for statuses
 - **Help File Menu Tab**:
   - A "Help" tab invoking About / Welcome windows has been added to file menu.
 - **Control Sizing**:
   - TODO
 - **MIDI Colors**:
   - MIDI notes in pianoroll displays are now colored according to instrument number.

 ### Requirements
 - Supports models deployed with [`pyharp` **v0.3.0**](https://github.com/TEAMuP-dev/pyharp/releases/tag/v0.3.0) and `gradio` **v5.x.x**.
