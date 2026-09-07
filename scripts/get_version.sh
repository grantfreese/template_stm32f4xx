#!/bin/bash
# Version generation script

set -e

# Check if we're in CI and have a tag
if [[ -n "$CI" && "$GITHUB_REF" == refs/tags/* ]]; then
    # CI build from tag: use clean semver
    VERSION=${GITHUB_REF#refs/tags/}
    # Remove 'v' prefix if present
    VERSION=${VERSION#v}
    echo "$VERSION"
    exit 0
fi

# Local build or CI build from branch
# Get the most recent tag. Fallback default is the pre-release default
# bump this when the repo is tagged for a new release.
LATEST_TAG=$(git describe --tags --abbrev=0 2>/dev/null || echo "0.1.0")

# Remove 'v' prefix if present
LATEST_TAG=${LATEST_TAG#v}

# Get number of commits since tag
COMMITS_SINCE=$(git rev-list --count ${LATEST_TAG}..HEAD 2>/dev/null || echo "0")

# Build suffix based on build type and changes
SUFFIX=""

# Check if this is a local build (not CI)
if [[ -z "$CI" ]]; then
    SUFFIX="${SUFFIX}L"
fi

# Check for uncommitted changes
if ! git diff-index --quiet HEAD --; then
    SUFFIX="${SUFFIX}D"
fi

# Add dash prefix if we have any suffixes
if [[ -n "$SUFFIX" ]]; then
    SUFFIX="-${SUFFIX}"
fi

# Build version string
if [[ "$COMMITS_SINCE" == "0" ]]; then
    VERSION="${LATEST_TAG}${SUFFIX}"
else
    VERSION="${LATEST_TAG}+${COMMITS_SINCE}${SUFFIX}"
fi

echo "$VERSION"
