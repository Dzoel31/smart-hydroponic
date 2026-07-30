# Agent Rules for Smart Hydroponic Project

- **Write Clean & Neat Code**: Always ensure that the code you write is clean, well-organized, and follows best practices for both Python (backend) and Vue/TypeScript (frontend). Keep the structure consistent with the existing codebase.
- **Lint & Format Reminder**: At the end of tasks that involve code changes, always remind the user to run the linter and formatter, or offer to run them automatically. 
  - For Backend: `uv run ruff format .` and `uv run ruff check . --fix`
  - For Frontend: `npx prettier --write .`

- **Follow Contributing Guidelines (Strict Commit Rules)**: 
  - ALWAYS use Conventional Commits (e.g., `feat(scope): message`, `fix(scope): message`).
  - Allowed scopes: `backend`, `frontend`, `frontend-ui`, `frontend-web`, `shared`, `infra`.
  - **CRITICAL**: Never mix backend and frontend changes in a single commit. They must be split into separate atomic commits with the correct scopes to prevent CI/CD release workflow conflicts.
  - Read `CONTRIBUTING.md` for full details if unsure.

- **Check Official Documentation**: If the project uses tools, frameworks, or libraries whose versions exceed your knowledge cut-off date (e.g., newer versions of Vue, FastAPI, or uv), ALWAYS use your web search tools to read the official documentation before implementing features or fixing bugs to avoid hallucinating deprecated APIs.
