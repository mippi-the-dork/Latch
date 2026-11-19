# Maintaining Latch Documentation

## Content ownership

Design pages teach how to use a feature, what it does, and expected exceptions. Technical pages explain classes, data structures, saved state, engine hooks and constraints. Source verification is separate from live Unreal Editor testing.

## Changes

1. Audit the current Latch C++ implementation before changing feature claims.
2. Update both Design and Technical pages for affected topics.
3. Update category tables and the home category list if adding/removing a topic. Keep existing URLs stable.
4. Update media captions and capture instructions if the interaction changes.
5. Validate category links, Markdown images, search, breadcrumb navigation and the Design/Technical switch.
6. Record actual tests in [Verification Matrix](VERIFICATION_MATRIX.md), not inferred results.

## Visual assets

Use the genuine [Latch controls screenshot](https://github.com/mippi-the-dork/Latch/blob/main/Doc/Images/Latch-Controls.png), [Keep Visible GIF](https://github.com/mippi-the-dork/Latch/blob/main/Doc/Images/Latch-Keep-Visible.gif), settings image and hero where they serve the explanation.

Planned media assets use the supplied neutral placeholder until captured. Keep **reader-facing captions** and the separate **Capture instructions · Image/GIF** line beneath each media slot. The [Media Inventory](MEDIA_INVENTORY.md) tracks intended filenames and exact capture directions.

## Publishing

Configure GitHub Pages under **Settings → Pages → Deploy from branch → main → /docs**. Expected URL: https://mippi-the-dork.github.io/Latch/ . Do not claim deployment is live until checked.

## Version notes

`Latch.uplugin` reports v2.0.3. The README contains a stale v1.0.18 compatibility row; any correction to the root README should be a separate, explicitly scoped change.
