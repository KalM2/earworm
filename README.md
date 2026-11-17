# Earworm

A smart audiobook player for the Omarchy desktop — inspired by Smart AudioBook Player on Android.

## Features

- **Library management** — point it at a folder; each subfolder with audio files becomes a book
- **Per-book progress** — remembers where you left off in every book
- **Book categories** — New, Started, Finished
- **Variable playback speed** — 0.25x to 4.0x, adjust with `[` and `]`
- **Chapter navigation** — auto-detects chapters from files, auto-advances
- **Sleep timer** — pause playback after 5–60 minutes
- **Bookmarks** — mark and label interesting moments
- **Cover art** — detects cover images in book folders
- **Metadata extraction** — reads title and author from audio file tags
- **Keyboard-first** — every action has a shortcut

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Space` | Play / Pause |
| `←` | Back 10 seconds |
| `→` | Forward 30 seconds |
| `P` | Previous chapter |
| `N` | Next chapter |
| `[` | Slow down |
| `]` | Speed up |
| `B` | Add bookmark |
| `S` | Sleep timer |
| `C` | Chapters view |
| `M` | Bookmarks view |
| `O` | Open folder |
| `Esc` | Back / Close |
| `?` | Show shortcuts |
| `Q` | Quit |

## Supported Formats

MP3, M4A, M4B, OGG, Opus, FLAC, WMA, WAV, AAC

## Building

```
bin/build
```

## Installing

```
bin/install
```

The app appears in the Omarchy launcher (`Super + Space`).

## License

MIT — includes patterns from Omacut (MIT, David Heinemeier Hansson).
