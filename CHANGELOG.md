\# 🧠 Changelog

All notable changes to \*\*Laboratory C Final Project — Two-Pass Assembler\*\* will be documented in this file.  

This project follows \[Keep a Changelog](https://keepachangelog.com/en/1.0.0/) guidelines and uses \[Semantic Versioning](https://semver.org/).



---



\## \[Unreleased]

\### Added

\- Continuous Integration (CI) via GitHub Actions

\- Clean `.gitignore` for a professional C setup

\- Comprehensive `README.md` with project structure and usage guide

\- `LICENSE` file (MIT License)

\- `CHANGELOG.md` for tracking project evolution



\### Planned

\- Automated unit testing and output verification via CI

\- Integration of second pass assembler logic

\- Extended documentation with flowcharts and error table



---



\## \[v1.0.0] — 2025-10-15

\### Added

\- Initial stable version of the \*\*Two-Pass Assembler\*\* in ANSI C

\- Modular design with clear separation between phases:

&nbsp; - `pre\_assembler.c` — handles macro expansion  

&nbsp; - `first\_pass.c` — builds symbol/instruction tables  

&nbsp; - `memory\_image.c`, `symbol\_table.c`, `macro\_table.c` — structured modules  

\- Error handling modules: `error\_list.c`, `errors.c`

\- Test files and makefile support



\### Changed

\- Improved code organization: moved source files to `src/` and headers to `include/`

\- Added documentation and in-code comments



\### Removed

\- Legacy folder versions (`000\_`, `001\_`, etc.) from tracked repository

\- Old redundant test or backup files



---



\## \[v0.1.0] — 2025-07-01

\### Added

\- Initial proof-of-concept assembler implementation

\- Macro handling and first-pass logic prototypes

\- Early test files for `.as` → `.am` conversion



