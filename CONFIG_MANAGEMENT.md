# Configuration File Management

## Overview

Configuration files (`.ini` and `.dat`) are now managed through a centralized `config/` folder containing example templates. The build system automatically copies these examples to the output directory during compilation.

## Directory Structure

```
ConnectServerCross/
├── config/                          # Configuration templates (version controlled)
│   ├── ConnectServer.ini.example   # Server configuration template
│   └── ServerList.dat.example      # Server list template
├── ConnectServer.ini               # Active config (gitignored, auto-generated)
├── ServerList.dat                  # Active config (gitignored, auto-generated)
└── build/
    └── [Debug|Release]/
        ├── ConnectServer.exe       # Compiled binary
        ├── ConnectServer.ini       # Copied from config/
        └── ServerList.dat          # Copied from config/
```

## Build System Integration

### CMake (Cross-Platform)

The `CMakeLists.txt` includes post-build commands that automatically copy configuration examples:

```cmake
add_custom_command(TARGET ConnectServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config/ConnectServer.ini.example
        $<TARGET_FILE_DIR:ConnectServer>/ConnectServer.ini
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config/ServerList.dat.example
        $<TARGET_FILE_DIR:ConnectServer>/ServerList.dat
)
```

### Build Scripts

Both `scripts/build.bat` (Windows) and `scripts/build.sh` (Linux) have been updated to copy configuration files from the `config/` folder to the build output directory.

**Windows:**
```batch
copy /Y "..\config\ConnectServer.ini.example" "%BUILD_TYPE%\ConnectServer.ini"
copy /Y "..\config\ServerList.dat.example" "%BUILD_TYPE%\ServerList.dat"
```

**Linux:**
```bash
cp ../config/ConnectServer.ini.example ./ConnectServer.ini
cp ../config/ServerList.dat.example ./ServerList.dat
```

## Usage

### For Developers

1. **First Time Setup:**
   - Clone the repository
   - Run the build script or CMake
   - Configuration files are automatically created from examples

2. **Modifying Configuration:**
   - Edit the generated `ConnectServer.ini` or `ServerList.dat` in the build output directory
   - Your changes are preserved (files are gitignored)
   - To share configuration changes, update the `.example` files in `config/`

3. **Updating Example Templates:**
   - Edit files in `config/` folder
   - Commit changes to version control
   - Other developers will get the updated templates on next build

### For Production Deployment

1. **Initial Deployment:**
   - Copy `config/*.example` files to your deployment directory
   - Rename them (remove `.example` extension)
   - Configure for your production environment

2. **Updates:**
   - When updating the binary, check if example files have changed
   - Merge any new settings into your production configuration
   - Keep your custom settings intact

## Benefits

### ✅ Version Control
- Example configurations are tracked in Git
- Active configurations are gitignored (no accidental commits of sensitive data)
- Easy to see configuration changes in pull requests

### ✅ Clean Repository
- No configuration files in root directory
- Clear separation between templates and active configs
- Reduced clutter in project structure

### ✅ Developer Experience
- Automatic configuration setup on build
- No manual file copying required
- Consistent configuration across team members

### ✅ Production Safety
- Production configs never accidentally committed
- Easy to identify which settings are defaults
- Clear documentation through example files

## Migration from Old Structure

If you have existing configuration files in the root directory:

1. **Backup your current configs:**
   ```bash
   cp ConnectServer.ini ConnectServer.ini.backup
   cp ServerList.dat ServerList.dat.backup
   ```

2. **Build the project:**
   - The build system will create new configs from examples
   
3. **Merge your custom settings:**
   - Compare your backup files with the new generated files
   - Copy over any custom settings you need

4. **Clean up (optional):**
   - The root directory `.ini` and `.dat` files can be removed
   - They are gitignored and will be regenerated on next build

## Troubleshooting

### Configuration files not copied during build

**CMake:**
- Ensure you're building the target (not just configuring)
- Check that `config/*.example` files exist
- Verify CMake output for copy commands

**Build Scripts:**
- Check that you're running from the correct directory
- Verify the `config/` folder path is correct
- Look for error messages in build output

### Configuration changes not taking effect

- Make sure you're editing the config in the build output directory
- Restart the server after making changes
- Check file permissions (files must be readable)

## Adding New Configuration Files

To add a new configuration file to the system:

1. **Create the example file:**
   ```bash
   # Create in config folder
   touch config/NewConfig.ini.example
   ```

2. **Update CMakeLists.txt:**
   ```cmake
   add_custom_command(TARGET ConnectServer POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
           ${CMAKE_CURRENT_SOURCE_DIR}/config/NewConfig.ini.example
           $<TARGET_FILE_DIR:ConnectServer>/NewConfig.ini
   )
   ```

3. **Update build scripts:**
   - Add copy commands to `scripts/build.bat`
   - Add copy commands to `scripts/build.sh`

4. **Update .gitignore:**
   ```
   NewConfig.ini
   !NewConfig.ini.example
   ```

## Best Practices

1. **Always use .example suffix** for template files in `config/`
2. **Document all settings** with comments in example files
3. **Provide sensible defaults** that work out of the box
4. **Never commit active configs** (without `.example` suffix)
5. **Update examples** when adding new configuration options
6. **Test the build** after modifying config management

## Related Files

- `CMakeLists.txt` - CMake build configuration
- `scripts/build.bat` - Windows build script
- `scripts/build.sh` - Linux build script
- `.gitignore` - Git ignore rules
- `config/` - Configuration templates directory
