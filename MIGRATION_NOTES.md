# Configuration Management Migration Notes

## Summary of Changes

The configuration file management system has been updated to use a centralized `config/` folder with automatic copying during build.

## What Changed

### ✅ Build System Updates

1. **CMakeLists.txt** (lines 186-197)
   - Added post-build commands to copy config examples
   - Automatically creates `ConnectServer.ini` and `ServerList.dat` in output directory

2. **scripts/build.bat** (lines 99-108)
   - Updated to copy from `config/` folder instead of root
   - Now uses `config/*.example` files as source

3. **scripts/build.sh** (lines 88-101)
   - Updated to copy from `config/` folder instead of root
   - Now uses `config/*.example` files as source

### 📁 File Structure

**Before:**
```
ConnectServerCross/
├── ConnectServer.ini.example    ← Duplicate
├── ServerList.dat.example       ← Duplicate
├── ConnectServer.ini            ← Gitignored
├── ServerList.dat               ← Gitignored
└── config/
    ├── ConnectServer.ini.example
    └── ServerList.dat.example
```

**After:**
```
ConnectServerCross/
├── config/
│   ├── ConnectServer.ini.example  ← Source of truth
│   └── ServerList.dat.example     ← Source of truth
├── ConnectServer.ini              ← Gitignored (auto-generated)
└── ServerList.dat                 ← Gitignored (auto-generated)
```

### 📝 Documentation Added

1. **CONFIG_MANAGEMENT.md** - Comprehensive guide for config management
2. **MIGRATION_NOTES.md** - This file
3. **scripts/cleanup_root_configs.bat** - Windows cleanup script
4. **scripts/cleanup_root_configs.sh** - Linux cleanup script
5. **SETUP_CONFIG_MANAGEMENT.md** (root) - Guide for other servers

## Files to Remove

The following files in the root directory are now redundant:

- ✅ `ConnectServer.ini.example` (exists in `config/`)
- ✅ `ServerList.dat.example` (exists in `config/`)

**Note:** These files are identical to those in `config/` folder and can be safely removed.

## How to Clean Up

### Option 1: Use Cleanup Script (Recommended)

**Windows:**
```batch
cd ConnectServerCross
scripts\cleanup_root_configs.bat
```

**Linux:**
```bash
cd ConnectServerCross
chmod +x scripts/cleanup_root_configs.sh
./scripts/cleanup_root_configs.sh
```

### Option 2: Manual Removal

**Windows:**
```batch
cd ConnectServerCross
del ConnectServer.ini.example
del ServerList.dat.example
```

**Linux:**
```bash
cd ConnectServerCross
rm ConnectServer.ini.example
rm ServerList.dat.example
```

## Verification Steps

After cleanup, verify the setup:

1. **Check config folder:**
   ```bash
   ls config/
   # Should show: ConnectServer.ini.example, ServerList.dat.example
   ```

2. **Build the project:**
   ```bash
   # Windows
   scripts\build.bat
   
   # Linux
   ./scripts/build.sh
   ```

3. **Verify output directory:**
   ```bash
   # Windows
   dir build\Release\
   
   # Linux
   ls build/
   ```
   
   Should contain:
   - `ConnectServer.exe` (or `ConnectServer` on Linux)
   - `ConnectServer.ini` (copied from config/)
   - `ServerList.dat` (copied from config/)

4. **Test the server:**
   ```bash
   # Windows
   cd build\Release
   ConnectServer.exe
   
   # Linux
   cd build
   ./ConnectServer
   ```

## Impact on Workflows

### Developer Workflow

**Before:**
1. Clone repository
2. Manually copy `*.example` files
3. Rename to remove `.example`
4. Build project

**After:**
1. Clone repository
2. Build project ← Config files auto-created!

### Configuration Changes

**Before:**
- Edit root directory config files
- Risk of committing sensitive data

**After:**
- Edit build output directory config files
- Root configs are gitignored
- Update `config/*.example` to share changes

### Production Deployment

**Before:**
- Copy config files from root
- Unclear which are templates vs active

**After:**
- Copy from `config/` folder (clear templates)
- Rename to remove `.example`
- Customize for production

## Rollback Instructions

If you need to rollback these changes:

1. **Restore old build scripts:**
   ```bash
   git checkout HEAD~1 -- scripts/build.bat scripts/build.sh
   ```

2. **Restore old CMakeLists.txt:**
   ```bash
   git checkout HEAD~1 -- CMakeLists.txt
   ```

3. **Copy example files back to root:**
   ```bash
   cp config/ConnectServer.ini.example ./
   cp config/ServerList.dat.example ./
   ```

## Next Steps

1. ✅ Test the build system thoroughly
2. ✅ Verify config files are copied correctly
3. ✅ Run cleanup script to remove duplicates
4. ⏳ Apply same pattern to other servers (ConnectServer, GameServer, etc.)
5. ⏳ Update main solution documentation

## Questions or Issues?

- Check `CONFIG_MANAGEMENT.md` for detailed documentation
- Review `SETUP_CONFIG_MANAGEMENT.md` for applying to other servers
- Test with both Debug and Release builds
- Verify on both Windows and Linux platforms

## Compatibility

- ✅ Windows (MSVC, CMake)
- ✅ Linux (GCC, Clang, CMake)
- ✅ Existing configs preserved (gitignored)
- ✅ Build scripts backward compatible
- ✅ No changes to runtime behavior

## Benefits Recap

1. **Cleaner repository** - No config files in root
2. **Automatic setup** - No manual copying needed
3. **Version control** - Templates tracked in Git
4. **Safety** - Active configs never committed
5. **Consistency** - Same pattern across all servers
6. **Documentation** - Clear examples for all settings

---

**Migration Date:** 2024
**Status:** ✅ Complete for ConnectServerCross
**Next:** Apply to other server components
