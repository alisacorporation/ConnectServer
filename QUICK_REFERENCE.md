# Configuration Management - Quick Reference

## 📍 File Locations

### Configuration Templates (Version Controlled)
```
config/
├── ConnectServer.ini.example  ← Edit to share changes
└── ServerList.dat.example     ← Edit to share changes
```

### Active Configurations (Gitignored)
```
build/Release/
├── ConnectServer.ini          ← Edit for local changes
└── ServerList.dat             ← Edit for local changes
```

## 🔨 Build Commands

### CMake (Recommended)
```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build (configs auto-copied)
cmake --build build --config Release

# Run
cd build/Release  # Windows
./build/ConnectServer  # Linux
```

### Build Scripts
```bash
# Windows
scripts\build.bat --release

# Linux
./scripts/build.sh --release
```

## 📝 Common Tasks

### First Time Setup
```bash
git clone <repository>
cd ConnectServerCross
scripts/build.bat  # or ./scripts/build.sh
# Configs automatically created!
```

### Modify Local Config
```bash
# Edit files in build output directory
notepad build\Release\ConnectServer.ini  # Windows
nano build/ConnectServer.ini             # Linux
```

### Share Config Changes
```bash
# Edit template files
notepad config\ConnectServer.ini.example
git add config/ConnectServer.ini.example
git commit -m "Update config template"
```

### Clean Build
```bash
# Windows
scripts\build.bat --clean --release

# Linux
./scripts/build.sh --clean --release
```

## 🗑️ Cleanup (One-time)

If you have duplicate `.example` files in root:

```bash
# Windows
scripts\cleanup_root_configs.bat

# Linux
./scripts/cleanup_root_configs.sh
```

## ⚠️ Important Rules

### ✅ DO
- Edit configs in **build output directory** for local changes
- Edit `config/*.example` files to share configuration changes
- Commit `config/*.example` files to Git
- Keep production configs secure

### ❌ DON'T
- Don't edit `config/*.example` for local testing
- Don't commit files without `.example` suffix
- Don't put sensitive data in `.example` files
- Don't manually copy config files (build does it)

## 🔍 Troubleshooting

### Config files not found
```bash
# Check if example files exist
ls config/

# Rebuild to regenerate configs
cmake --build build --config Release
```

### Changes not taking effect
```bash
# Make sure you're editing the right file
# Edit: build/Release/ConnectServer.ini
# NOT: config/ConnectServer.ini.example
```

### Build fails
```bash
# Verify config folder exists
ls config/

# Check CMakeLists.txt has post-build commands
grep "copy_if_different" CMakeLists.txt
```

## 📚 Full Documentation

- **CONFIG_MANAGEMENT.md** - Complete guide
- **MIGRATION_NOTES.md** - Migration details
- **BUILD_CONFIG_SUMMARY.md** - Implementation summary
- **../SETUP_CONFIG_MANAGEMENT.md** - Guide for other servers

## 🎯 Quick Checklist

Building for the first time?
- [ ] Clone repository
- [ ] Run build script
- [ ] Configs auto-created ✓
- [ ] Run server

Modifying configuration?
- [ ] Edit file in build output directory
- [ ] Test changes
- [ ] If sharing: update `config/*.example`
- [ ] Commit example file

Cleaning up old files?
- [ ] Run cleanup script
- [ ] Verify config/ folder intact
- [ ] Rebuild project
- [ ] Test server

---

**Need help?** Check the full documentation in CONFIG_MANAGEMENT.md
