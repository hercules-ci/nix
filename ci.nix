args@{ ... }:
let
  release = import ./release.nix (args // {
    nix = { outPath = ./.; revCount = 0; shortRev = "dev"; };
  });

  inherit (builtins) typeOf isAttrs mapAttrs;
  isDerivation = x: isAttrs x && x ? type && x.type == "derivation";

  # Hercules CI emulates nix-build behavior, so we need to recurseIntoAttrs.
  # See https://docs.hercules-ci.com/hercules-ci/reference/evaluation/
  recursiveRecurseIntoAttrs = a:
    if isAttrs a
       && !isDerivation a
       && a.recurseForDerivations or true # true: Hydra does recurse by default
    then mapAttrs (k: recursiveRecurseIntoAttrs) a // { recurseForDerivations = true; }
    else a;

in
  recursiveRecurseIntoAttrs release
