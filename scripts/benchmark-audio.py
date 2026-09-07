#!/usr/bin/env python3
"""Compare the same render workload against the pre-quality engine on this runner."""
import io, pathlib, subprocess, tempfile, zipfile
root = pathlib.Path(__file__).resolve().parents[1]
base = '0873064487e02b2e1da49b42d0fd12ca83e11ec7'
def run(*args, cwd=root):
    return subprocess.check_output(args, cwd=cwd, text=True)
def executable(build):
    return next(p for p in build.rglob('sawstar_audio_benchmark*') if p.is_file() and (p.suffix == '.exe' or not p.suffix))
with tempfile.TemporaryDirectory(prefix='sawstar-benchmark-') as temp:
    source = pathlib.Path(temp) / 'baseline'; source.mkdir()
    data = subprocess.check_output(['git','archive','--format=zip',base],cwd=root)
    with zipfile.ZipFile(io.BytesIO(data)) as archive: archive.extractall(source)
    # Use the exact same pinned dependency and workload for both engines.
    cmake = (source/'CMakeLists.txt').read_text().replace('${CMAKE_CURRENT_SOURCE_DIR}/third_party/DaisySP', (root/'third_party/DaisySP').as_posix())
    cmake += '\nadd_executable(sawstar_audio_benchmark tests/audio_benchmark.cpp)\ntarget_link_libraries(sawstar_audio_benchmark PRIVATE sawstar_engine)\n'
    (source/'CMakeLists.txt').write_text(cmake)
    (source/'tests/audio_benchmark.cpp').write_text((root/'tests/audio_benchmark.cpp').read_text())
    build=pathlib.Path(temp)/'build'
    run('cmake','-S',str(source),'-B',str(build),'-DCMAKE_BUILD_TYPE=Release','-DSAWSTAR_CHECK_DAISYSP=ON','-DBUILD_TESTING=OFF')
    run('cmake','--build',str(build),'--config','Release','--target','sawstar_audio_benchmark','--parallel','2')
    for repeat in range(3):
        for label, folder in [('baseline',build),('current',root/'build')]:
            print(f'{label},repeat={repeat+1}',flush=True)
            print(run(str(executable(folder))),flush=True)
