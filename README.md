# ALComplete

**Mod ID:** `bedrock.alcomplete.geode`  
**Target game:** Geometry Dash 2.2081  
**Build targets:** Android32 (`armeabi-v7a`) and Android64 (`arm64-v8a`)

## GitHub Actions

Push this project to a GitHub repository, then open **Actions → Build ALComplete (Android 32-bit + 64-bit)**. It builds the two Android targets independently and uploads a separate artifact for each target.

## Local build

Install the Geode CLI, matching SDK/binaries, and Android NDK, then run:

```sh
geode sdk install-binaries -p android32
geode build -p android32

geode sdk install-binaries -p android64
geode build -p android64
```

## Status

This is a build-focused starter implementation, not the complete UI in the original brief. It currently includes a floating AL button, a quick-action popup, and settings for opacity, scale, label, and visibility. The button is not yet draggable, and a custom categorized in-game menu is not yet implemented.

The completion action calls `PlayLayer::levelComplete()` directly; it does not replay inputs. Behavior can differ from naturally completing a level. Compile and test against the exact Geode SDK installed in your environment before distributing.