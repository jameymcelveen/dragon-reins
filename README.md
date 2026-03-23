dragon-reins/
├── apps/
│ └── electron-ui/ # The Shared UI (TS/React/Vue)
├── packages/
│ ├── bridge-macos/ # The Swift App + C++ DriverKit Target
│ │ └── DragonReins.xcodeproj
│ ├── bridge-windows/ # C++ / C# WinUSB / HID implementation
│ │ └── DragonReins.sln
│ └── / # JSON/TS constants for #F56600 logic
├── package.json # Root-level orchestrator
└── README.md
