#!/usr/bin/env bash
set -euo pipefail

workspace_dir="$(cd "$(dirname "$0")/.." && pwd)"
toolchain_dir="/Users/nops/Code/SimTower"
ghidra_dir="$toolchain_dir/ghidra_12.1.4_PUBLIC"
mcp_jar="$toolchain_dir/ghidra-mcp/build/libs/GhidraMCP-7.0.0.jar"
profile_dir="$workspace_dir/.ghidra-profile"

if [[ ! -x /opt/homebrew/opt/openjdk@21/bin/java ]]; then
  echo "OpenJDK 21 is required at /opt/homebrew/opt/openjdk@21/bin/java" >&2
  exit 1
fi

if [[ ! -f "$mcp_jar" ]]; then
  echo "GhidraMCP is missing: $mcp_jar" >&2
  exit 1
fi

mkdir -p "$profile_dir"

classpath="$mcp_jar"
for jar in \
  "$ghidra_dir"/Ghidra/Framework/*/lib/*.jar \
  "$ghidra_dir"/Ghidra/Features/*/lib/*.jar \
  "$ghidra_dir"/Ghidra/Processors/*/lib/*.jar; do
  [[ -f "$jar" ]] && classpath="$classpath:$jar"
done

exec /opt/homebrew/opt/openjdk@21/bin/java \
  -Xmx4g \
  -Duser.home="$profile_dir" \
  -Dghidra.home="$ghidra_dir" \
  -Dapplication.name=GhidraMCP \
  -classpath "$classpath" \
  com.xebyte.headless.GhidraMCPHeadlessServer \
  --bind 127.0.0.1 \
  --port 8089 \
  "$@"
