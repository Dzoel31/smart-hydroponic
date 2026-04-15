# Maintainer Readiness Rubric

This rubric helps decide whether a contributor is ready to become a maintainer for this backend.

## Goal

A maintainer can safely own the project from development to deployment, including Docker-based operations.

## Levels

- `L1` Beginner: can contribute with guidance.
- `L2` Contributor: can deliver features/bug fixes independently.
- `L3` Maintainer-ready: can own releases and incidents.

## Core Competencies

### 1. Python Fundamentals

Minimum:

- Understands functions, classes, exceptions, and typing basics.
- Reads and writes async Python code (`async`/`await`).
- Can reason about Pydantic models and validation.

Maintainer-ready signals:

- Writes clean, typed code with clear error handling.
- Can debug stack traces and isolate root causes quickly.

### 2. FastAPI and API Design

Minimum:

- Understands routes, dependencies, request/response models, and status codes.
- Understands auth flow (`Bearer`, token decode/validate).

Maintainer-ready signals:

- Designs backward-compatible API changes.
- Enforces RBAC/authorization rules consistently.
- Reviews API behavior critically (security, semantics, edge cases).

### 3. Data Layer and Migrations

Minimum:

- Understands SQL basics and current raw SQL pattern in this repo.
- Can read migration history and run Alembic upgrades.

Maintainer-ready signals:

- Prevents schema/code drift.
- Handles migration rollback strategy for failed deploys.
- Investigates and resolves integrity/constraint failures correctly.

### 4. Testing and Quality

Minimum:

- Can write/maintain pytest tests for success and failure paths.
- Can run project quality checks.

Maintainer-ready signals:

- Adds tests for logic/risk changes before merging.
- Keeps coverage meaningful for core flows (auth, RBAC, data access).
- Uses lint/format tooling consistently.

Required commands:

- `uv run --group dev pytest test/test_api_fastapi.py test/test_api_fastapi_edge.py -q`
- `uv run --group dev ruff check .`
- `uv run --group dev ruff format .`

### 5. Docker and Environment Operations

Minimum:

- Can run app and database using Docker/Docker Compose.
- Understands `.env` variables and secrets handling basics.

Maintainer-ready signals:

- Can troubleshoot container networking/config issues.
- Can validate health checks and startup dependencies.
- Can perform safe deploy/rollback using containerized workflow.

Minimum practical tasks:

- Build image from `Dockerfile`.
- Start services with `docker-compose.yml`.
- Verify app health endpoint after startup.

### 6. Git and Collaboration

Minimum:

- Uses branch workflow and small, reviewable commits.
- Writes clear PR descriptions.

Maintainer-ready signals:

- Reviews code effectively and gives actionable feedback.
- Manages release/hotfix branches responsibly.

### 7. Incident Response and Ownership

Minimum:

- Can read logs and reproduce bugs.

Maintainer-ready signals:

- Handles production incidents with calm triage:
  1. detect
  2. mitigate
  3. communicate
  4. follow-up
- Writes short postmortem notes with prevention actions.

## Readiness Checklist (Pass/Fail)

A maintainer candidate should pass all items below:

1. Can implement a feature touching route + service + tests without breaking existing flows.
2. Can enforce authorization changes safely and add regression tests.
3. Can run migrations and explain current DB schema impact.
4. Can run and pass lint/format/tests locally via `uv`.
5. Can run app with Docker and verify health/readiness.
6. Can diagnose and fix at least one failing test and one runtime error.
7. Can prepare a release candidate PR with risk notes and rollback plan.

## Suggested Handover Plan (4-6 Weeks)

1. Week 1-2 (Shadow)

    Candidate pairs with current maintainer on review + bug triage.

2. Week 3-4 (Co-own)

    Candidate leads one feature and one bugfix release under supervision.

3. Week 5-6 (Lead)

    Candidate leads sprint maintenance (merge queue, release prep, incident on-call backup).

Promotion decision:

- Promote to maintainer if candidate consistently demonstrates L3 behavior and passes checklist.
