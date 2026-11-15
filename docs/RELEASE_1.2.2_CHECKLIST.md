# SAWSTAR 1.2.2 publication checklist

- [x] Creator authorized a new 1.2.2 correction release, rather than replacing 1.2.1 assets.
- [x] Public creator identity applied to authored documents and generated package metadata.
- [x] Stable 1.2.2 metadata and version-specific notes prepared.
- [x] Regenerated bilingual manuals validated for text, metadata and page layout.
- [x] Exact final PR head: all relevant workflows/jobs and review threads clear.
- [x] Squash merge with expected head SHA; all exact-main push workflows/jobs pass.
- [x] Verified private draft: three platforms, seven packages and checksum list;
      ZIP CRC/inventory, payload hashes, embedded version/SHA, PDF bytes and links.
- [x] Publish the verified draft using the explicit 13 November authorization;
      verify public Latest, tag target and unchanged verified asset digests.

K1 and automation limitations remain disclosed. Prior RC host feedback does not
establish exact-1.2.2 or native Windows ARM64 host acceptance. Historical raw
evidence, tags and package bytes are retained. No favorable-result retry or
threshold relaxation is authorized by this checklist.

## Publication evidence

Published [v1.2.2](https://github.com/RobCZart82/SAWSTAR/releases/tag/v1.2.2)
on 13 November 2026 at 22:42:38 UTC. Expected-head squash merge of
[PR #152](https://github.com/RobCZart82/SAWSTAR/pull/152) produced
`cd8785fb428bb94ff3fc1a62381c5294bf01c848`. All six exact-main workflows and
13 jobs succeeded: Windows 43515697110, macOS 43515697173, quality 43515697231,
lint 43515697370, host-kit preflight 43515697296, draft preparation 43515697475.
All eight public assets matched the verified draft; the tag targets that commit.
The 1.2.1 package bytes were retained. These results apply to that frozen build,
not to later changes. No new native host or K1-fix acceptance is inferred.
