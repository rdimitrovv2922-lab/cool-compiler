#!/usr/bin/env bash
set -uo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
root_dir="$(cd -- "${script_dir}/.." && pwd)"
tests_dir="${root_dir}/tests/lexer"
lexer="${root_dir}/build/lexer"

if [[ ! -x "${lexer}" ]]; then
    echo "Lexer executable not found; run bash tools/build.sh first." >&2
    exit 2
fi

shopt -s nullglob
inputs=("${tests_dir}"/*.in)
if (( ${#inputs[@]} == 0 )); then
    echo "No lexer tests found in ${tests_dir}" >&2
    exit 2
fi

tmp_dir="$(mktemp -d "${TMPDIR:-/tmp}/cool-lexer-tests.XXXXXX")" || exit 2
trap 'rm -rf -- "${tmp_dir}"' EXIT
total=0
failed=0

for input in "${inputs[@]}"; do
    total=$((total + 1))
    name="$(basename -- "${input%.in}")"
    actual="${tmp_dir}/${name}.out"
    expected="${tests_dir}/${name}.out"

    if ! "${lexer}" < "${input}" > "${actual}" 2>&1; then
        echo "${name}: lexer exited with a failure status"
        failed=$((failed + 1))
    elif ! diff -u -- "${expected}" "${actual}"; then
        echo "${name}: FAIL"
        failed=$((failed + 1))
    fi
done

passed=$((total - failed))
echo "Lexer: ${passed}/${total} tests passed."
(( failed == 0 ))
