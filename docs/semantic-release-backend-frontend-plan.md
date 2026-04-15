# Semantic Release Split Plan (Backend + Frontend)

Date: 2026-04-07
Repository: IoT-Smart-Hydroponic/smart-hydroponic
Branch reviewed: dev-python

## Objective

Run semantic release safely for both backend and frontend in one repository, without version collisions, wrong bumps, or cross-component release noise.

## Current State (Audit Summary)

1. Backend release exists and runs on every push to dev-python.
2. Backend semantic-release config exists in backend/pyproject.toml.
3. Frontend release workflow is not present yet.
4. Backend release tags currently use generic vX.Y.Z format.

## Main Risks Found

1. Cross-component bump risk
    - Backend release is branch-triggered and not path-filtered.
    - Frontend-only commits can still trigger backend bump logic.

2. Tag namespace collision risk
    - If frontend also uses generic vX.Y.Z tags, both pipelines can target same tag names.

3. Release build traceability risk
    - Docker release workflow uses a ref expression from a different event type.
    - Potential mismatch between release tag and checked-out commit.

## Target Design (Safe)

1. Isolated tag namespaces
    - Backend tag format: backend-v{version}
    - Frontend tag format: frontend-v{version}

2. Path-scoped release workflows
    - Backend release runs only when backend/** (and backend release workflow) changes.
    - Frontend release runs only when frontend-vue/** (and frontend release workflow) changes.

3. Independent changelogs and version sources
    - Backend keeps backend/CHANGELOG.md and backend/pyproject.toml version.
    - Frontend keeps frontend-vue/CHANGELOG.md and frontend-vue/package.json version.

4. Clear commit convention
    - Use Conventional Commits with explicit scope.
    - Examples:
        - feat(backend): add role guard for admin endpoints
        - fix(frontend): handle expired token redirect

5. Deterministic release builds
    - Docker workflow checks out release tag ref directly.

## Required Secrets and Permissions

Required repository secrets:

1. RELEASE_TOKEN
    - GitHub token with contents:write for tagging/release notes.
2. DOCKER_USERNAME
3. DOCKER_PUSH

Recommended permissions in release jobs:

- contents: write

## Proposed File Changes

### 1) .github/workflows/release-backend.yml

Add path filters under push:

```yaml
on:
  push:
    branches:
      - dev-python
    paths:
      - 'backend/**'
      - '.github/workflows/release-backend.yml'
```

Keep working-directory as backend.

### 2) backend/pyproject.toml

Set backend-specific tag format in tool.semantic_release section:

```toml
[tool.semantic_release]
tag_format = "backend-v{version}"
```

Keep existing branch prerelease behavior for dev-python.

### 3) New frontend semantic-release config

Create frontend-vue/.releaserc.json:

```json
{
  "branches": [
    "main",
    {
      "name": "dev-python",
      "prerelease": "beta"
    }
  ],
  "tagFormat": "frontend-v${version}",
  "plugins": [
    [
      "@semantic-release/commit-analyzer",
      {
        "preset": "conventionalcommits"
      }
    ],
    [
      "@semantic-release/release-notes-generator",
      {
        "preset": "conventionalcommits"
      }
    ],
    [
      "@semantic-release/changelog",
      {
        "changelogFile": "CHANGELOG.md"
      }
    ],
    [
      "@semantic-release/npm",
      {
        "npmPublish": false
      }
    ],
    [
      "@semantic-release/git",
      {
        "assets": [
          "package.json",
          "CHANGELOG.md"
        ],
        "message": "chore(release): frontend-v${nextRelease.version} [skip ci]"
      }
    ],
    "@semantic-release/github"
  ]
}
```

### 4) New workflow .github/workflows/release-frontend.yml

```yaml
name: Frontend Release

on:
  push:
    branches:
      - dev-python
    paths:
      - 'frontend-vue/**'
      - '.github/workflows/release-frontend.yml'
  workflow_dispatch:

jobs:
  release:
    if: github.repository == 'IoT-Smart-Hydroponic/smart-hydroponic' && github.actor != 'github-actions[bot]'
    runs-on: ubuntu-latest
    concurrency:
      group: release-frontend-dev-python
      cancel-in-progress: false
    permissions:
      contents: write
    defaults:
      run:
        working-directory: frontend-vue
    steps:
      - name: Checkout
        uses: actions/checkout@v4
        with:
          fetch-depth: 0

      - name: Setup Node
        uses: actions/setup-node@v4
        with:
          node-version: '22'

      - name: Install dependencies
        run: npm ci

      - name: Run semantic-release
        env:
          GITHUB_TOKEN: ${{ secrets.RELEASE_TOKEN }}
        run: npx semantic-release
```

### 5) frontend-vue/package.json

Add dev dependencies for frontend semantic release:

```json
{
  "devDependencies": {
    "semantic-release": "^24.2.0",
    "@semantic-release/changelog": "^6.0.3",
    "@semantic-release/commit-analyzer": "^13.0.1",
    "@semantic-release/git": "^10.0.1",
    "@semantic-release/github": "^11.0.1",
    "@semantic-release/npm": "^12.0.1",
    "@semantic-release/release-notes-generator": "^14.0.3"
  }
}
```

### 6) .github/workflows/docker-push-backend.yml

Use release tag ref for checkout in release event:

```yaml
- name: Checkout
  uses: actions/checkout@v4
  with:
    fetch-depth: 0
    fetch-tags: true
    ref: ${{ github.event.release.tag_name }}
```

Note:

- If checkout expects full ref syntax in your environment, use refs/tags/${{ github.event.release.tag_name }}.

## Optional Hardening (Recommended)

1. Add workflow-level branch protection rules
    - Require status checks for release workflows before merge.

2. Add commit lint in CI
    - Enforce Conventional Commits and valid scope values (backend/frontend).

3. Add path-aware CI matrix
    - Backend tests only when backend files change.
    - Frontend tests only when frontend files change.

4. Add release dry-run workflows
    - Manual workflow dispatch to test bump logic with --dry-run.

## Rollout Plan

Phase 1 (safe prep)

1. Add frontend release config and workflow.
2. Add path filters to backend release workflow.
3. Add backend tag_format.
4. Fix docker checkout ref.

Phase 2 (validation)

1. Push docs-only change under backend and verify backend release no-op/expected behavior.
2. Push docs-only change under frontend-vue and verify frontend release no-op/expected behavior.
3. Push feat(backend) commit and verify only backend tag is created.
4. Push feat(frontend) commit and verify only frontend tag is created.

Phase 3 (stabilization)

1. Monitor first 3 release cycles.
2. Freeze any further release tooling changes during observation period.

## Validation Checklist

1. Tags are namespaced:
    - backend-v*
    - frontend-v*
2. A frontend-only commit does not trigger backend release.
3. A backend-only commit does not trigger frontend release.
4. Release notes/changelog are updated in correct directory only.
5. Docker image tag aligns to backend release tag.

## Rollback Plan

If release behavior is wrong:

1. Disable release workflows temporarily (workflow_dispatch only).
2. Revert release config commits.
3. Delete incorrect pre-release tags if needed.
4. Re-run release in dry-run mode and validate logs before re-enabling push trigger.

## What I Need From You Later

1. Decision on final tag naming:

    - Option A: backend-v* and frontend-v* (recommended)
    - Option B: v* for one app and prefixed tags for the other (not recommended)

2. Confirm whether frontend should release from dev-python as beta and main as stable.

3. Confirm if frontend artifacts should be uploaded to GitHub Release or only version/changelog update.

4. Confirm if Docker release should be backend only (current expectation).

## Notes

- This plan is designed to avoid crash versioning in a monorepo.
- The most critical controls are path filters and tag namespace isolation.
- Once implemented, backend and frontend can release independently with predictable version history.
