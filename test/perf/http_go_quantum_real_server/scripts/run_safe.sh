#!/usr/bin/env bash
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_ROOT="$(cd "${ROOT}/../../../../.." && pwd)"
QTLC_BIN="${QTLC_BIN:-${REPO_ROOT}/qtlc/build/compiler/driver/qtlc}"

REQUESTS="${QTLC_HTTP_BENCH_REQUESTS:-256}"
CONCURRENCY="${QTLC_HTTP_BENCH_CONCURRENCY:-8}"
GO_PORT="${QTLC_HTTP_BENCH_GO_PORT:-18080}"
QUANTUM_PORT="${QTLC_HTTP_BENCH_QUANTUM_PORT:-18081}"
REQUIRE_QUANTUM="${QTLC_HTTP_BENCH_REQUIRE_QUANTUM:-0}"
REQUIRE_WIN="${QTLC_HTTP_BENCH_REQUIRE_WIN:-0}"

BUILD_DIR="${ROOT}/build"
REPORT_DIR="${ROOT}/reports"
REPORT="${REPORT_DIR}/latest.txt"
mkdir -p "${BUILD_DIR}" "${REPORT_DIR}"

export GOCACHE="${BUILD_DIR}/gocache"
export GOMODCACHE="${BUILD_DIR}/gomodcache"
mkdir -p "${GOCACHE}" "${GOMODCACHE}"

GO_SERVER_PID=""
QUANTUM_SERVER_PID=""

cleanup() {
  if [[ -n "${GO_SERVER_PID}" ]]; then
    kill "${GO_SERVER_PID}" >/dev/null 2>&1 || true
  fi
  if [[ -n "${QUANTUM_SERVER_PID}" ]]; then
    kill "${QUANTUM_SERVER_PID}" >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT

write_report_header() {
  {
    echo "benchmark=http-go-vs-quantum-real-server"
    echo "route=GET /users"
    echo "response={\"users\":[{\"id\":1,\"name\":\"salam\"}]}"
    echo "requests=${REQUESTS}"
    echo "concurrency=${CONCURRENCY}"
    echo "goPort=${GO_PORT}"
    echo "quantumPort=${QUANTUM_PORT}"
    echo "qtlc=${QTLC_BIN}"
  } > "${REPORT}"
}

append_report() {
  echo "$*" >> "${REPORT}"
}

extract_metric() {
  local text="$1"
  local key="$2"
  echo "${text}" | sed -n "s/.*${key}=\\([^ ]*\\).*/\\1/p" | tail -n 1
}

wait_http() {
  local url="$1"
  local expect="$2"
  local tries=40
  local i=0
  while [[ "${i}" -lt "${tries}" ]]; do
    local out
    out="$("${BUILD_DIR}/http-loadgen" -name health -url "${url}" -requests 1 -concurrency 1 -timeout-ms 500 -expect "${expect}" 2>/dev/null)"
    if echo "${out}" | grep -q "ok=true"; then
      return 0
    fi
    sleep 0.1
    i=$((i + 1))
  done
  return 1
}

find_quantum_executable() {
  find "${ROOT}/quantum_server/build" "${ROOT}/quantum_server/qtlc_artifacts" "${ROOT}/quantum_server" \
    -maxdepth 4 -type f -perm -111 2>/dev/null \
    | grep -E '/(quantum_http_real_server_benchmark|qtlc.out|main)$' \
    | head -n 1
}

write_report_header

echo "building go server and load generator"
(cd "${ROOT}/go_server" && go build -o "${BUILD_DIR}/go-server" .)
go_status=$?
(cd "${ROOT}/bench" && go build -o "${BUILD_DIR}/http-loadgen" .)
bench_status=$?
if [[ "${go_status}" -ne 0 || "${bench_status}" -ne 0 ]]; then
  append_report "goBuild=failed"
  echo "report=${REPORT}"
  exit 1
fi
append_report "goBuild=ok"
append_report "loadgenBuild=ok"

echo "building quantum server project"
if [[ -x "${QTLC_BIN}" ]]; then
  "${QTLC_BIN}" build "${ROOT}/quantum_server" --backend native > "${BUILD_DIR}/quantum-build.log" 2>&1
  quantum_build_status=$?
else
  quantum_build_status=127
  echo "qtlc not executable: ${QTLC_BIN}" > "${BUILD_DIR}/quantum-build.log"
fi
append_report "quantumBuildExit=${quantum_build_status}"
append_report "quantumBuildLog=${BUILD_DIR}/quantum-build.log"

PORT="${GO_PORT}" "${BUILD_DIR}/go-server" > "${BUILD_DIR}/go-server.log" 2>&1 &
GO_SERVER_PID="$!"
if ! wait_http "http://127.0.0.1:${GO_PORT}/health" "ok"; then
  append_report "goStatus=failed"
  append_report "goLog=${BUILD_DIR}/go-server.log"
  echo "report=${REPORT}"
  exit 1
fi
append_report "goStatus=running"

go_result="$("${BUILD_DIR}/http-loadgen" \
  -name go \
  -url "http://127.0.0.1:${GO_PORT}/users" \
  -requests "${REQUESTS}" \
  -concurrency "${CONCURRENCY}" \
  -expect "\"users\"")"
append_report "goResult=${go_result}"

if [[ "${quantum_build_status}" -ne 0 ]]; then
  append_report "quantumStatus=blocked"
  append_report "quantumBlockedReason=quantum-build-failed"
  append_report "claim=blocked"
  append_report "claimReason=quantum-server-build-failed-before-real-listener"
  cat "${REPORT}"
  if [[ "${REQUIRE_QUANTUM}" == "1" || "${REQUIRE_WIN}" == "1" ]]; then
    exit 2
  fi
  exit 0
fi

quantum_exe="$(find_quantum_executable)"
if [[ -z "${quantum_exe}" ]]; then
  append_report "quantumStatus=blocked"
  append_report "quantumBlockedReason=no-runnable-executable-produced"
  append_report "claim=blocked"
  append_report "claimReason=quantum-server-did-not-produce-real-listener"
  cat "${REPORT}"
  if [[ "${REQUIRE_QUANTUM}" == "1" || "${REQUIRE_WIN}" == "1" ]]; then
    exit 2
  fi
  exit 0
fi
append_report "quantumExecutable=${quantum_exe}"

"${quantum_exe}" > "${BUILD_DIR}/quantum-server.log" 2>&1 &
QUANTUM_SERVER_PID="$!"
sleep 0.2
if ! kill -0 "${QUANTUM_SERVER_PID}" >/dev/null 2>&1; then
  append_report "quantumStatus=blocked"
  append_report "quantumBlockedReason=process-exited-before-listening"
  append_report "quantumLog=${BUILD_DIR}/quantum-server.log"
  append_report "claim=blocked"
  append_report "claimReason=quantum-http-run-returned-without-real-server"
  cat "${REPORT}"
  if [[ "${REQUIRE_QUANTUM}" == "1" || "${REQUIRE_WIN}" == "1" ]]; then
    exit 2
  fi
  exit 0
fi

if ! wait_http "http://127.0.0.1:${QUANTUM_PORT}/health" "ok"; then
  append_report "quantumStatus=blocked"
  append_report "quantumBlockedReason=health-check-failed"
  append_report "quantumLog=${BUILD_DIR}/quantum-server.log"
  append_report "claim=blocked"
  append_report "claimReason=quantum-server-not-accepting-loopback-http"
  cat "${REPORT}"
  if [[ "${REQUIRE_QUANTUM}" == "1" || "${REQUIRE_WIN}" == "1" ]]; then
    exit 2
  fi
  exit 0
fi

append_report "quantumStatus=running"
quantum_result="$("${BUILD_DIR}/http-loadgen" \
  -name quantum \
  -url "http://127.0.0.1:${QUANTUM_PORT}/users" \
  -requests "${REQUESTS}" \
  -concurrency "${CONCURRENCY}" \
  -expect "\"users\"")"
append_report "quantumResult=${quantum_result}"

go_rps="$(extract_metric "${go_result}" "requestsPerSecond")"
quantum_rps="$(extract_metric "${quantum_result}" "requestsPerSecond")"
ratio="$(awk -v q="${quantum_rps}" -v g="${go_rps}" 'BEGIN { if (g > 0) printf "%.4f", q / g; else printf "0" }')"
append_report "rpsRatioQuantumOverGo=${ratio}"

win="$(awk -v r="${ratio}" 'BEGIN { if (r >= 1.10) print "true"; else print "false" }')"
if [[ "${win}" == "true" ]]; then
  append_report "claim=passed"
  append_report "claimReason=quantum-rps-at-least-10-percent-higher-than-go"
  cat "${REPORT}"
  exit 0
fi

append_report "claim=failed"
append_report "claimReason=quantum-rps-did-not-reach-10-percent-higher-than-go"
cat "${REPORT}"
if [[ "${REQUIRE_WIN}" == "1" ]]; then
  exit 3
fi
exit 0
