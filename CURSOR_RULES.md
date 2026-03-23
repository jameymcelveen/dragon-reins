# 🐅 Cursor DriverKit Rules for DragonReins

To successfully build and stage the `DragonReinsDriver` (dext), Cursor must follow these 6 strict constraints:

### 1. Header Ownership (.iig vs .h)

The `.iig` (IIG) file is the source of truth.

- **Rule:** Do NOT manually create or edit the class header file (e.g., `DragonReinsDriver.h`).
- **Rule:** Let the compiler generate the `.h` from the `.iig`. If a manual header exists with the same name as the class, delete it to prevent linker "Duplicate Symbol" errors.

### 2. The "IMPL" Macro Requirement

DriverKit does not use standard `Class::Method` syntax for implementations.

- **Rule:** Every method declared in the `.iig` MUST be implemented in the `.cpp` using the `IMPL(ClassName, MethodName)` macro.
- **Example:** `bool IMPL(DragonReinsDriver, Start) { ... }`

### 3. DeviceRequest Signature (8 Arguments)

The `USBDriverKit` framework version of `DeviceRequest` is different from legacy IOKit.

- **Rule:** Use exactly **8 arguments**: `bmRequestType`, `bRequest`, `wValue`, `wIndex`, `wLength`, `completion`, `completionTimeout`, and `completionStatus`.
- **Rule:** Do not attempt to use the 5 or 6-argument versions found in older tutorials.

### 4. Memory Management (No new/delete)

DriverKit runs in a restricted memory environment.

- **Rule:** Never use the `new` or `delete` keywords.
- **Rule:** Use the static `::Create` factory methods for objects (e.g., `IOBufferMemoryDescriptor::Create`).
- **Rule:** Use `OSSafeReleaseNULL(object)` to clean up.

### 5. Logging (No printf/std::cout)

Standard C++ output streams do not exist in the DriverKit user-space environment.

- **Rule:** Use `IOLog()` for basic logging or `os_log()` for more advanced tracing.
- **Rule:** `printf`, `std::cout`, and `fprintf` will cause immediate build failure.

### 6. Staging & Embedding

The driver must be physically "tucked" inside the App bundle to be seen by the OS.

- **Rule:** Ensure the `.dext` build product is copied to `DragonReins.app/Contents/Library/SystemExtensions/`.
- **Rule:** The `Info.plist` key `OSSystemExtensionCategoryIdentifier` must be set to `com.apple.developer.driverkit`.
