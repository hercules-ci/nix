# Release X.Y (202?-??-??)

- Two new builtin functions,
  [`builtins.parseFlakeRef`](@docroot@/language/builtins.md#builtins-parseFlakeRef)
  and
  [`builtins.flakeRefToString`](@docroot@/language/builtins.md#builtins-flakeRefToString),
  have been added.
  These functions are useful for converting between flake references encoded as attribute sets and URLs.

- [`builtins.toJSON`](@docroot@/language/builtins.md#builtins-parseFlakeRef) now prints [--show-trace](@docroot@/command-ref/conf-file.html#conf-show-trace) items for the path in which it finds an evaluation error.

- Error messages regarding malformed input to [`derivation add`](@docroot@/command-ref/new-cli/nix3-derivation-add.md) are now clearer and more detailed.

- The `discard-references` feature has been stabilized.
  This means that the
  [unsafeDiscardReferences](@docroot@/contributing/experimental-features.md#xp-feature-discard-references)
  attribute is no longer guarded by an experimental flag and can be used
  freely.

- The JSON output for derived paths with are store paths is now a string, not an object with a single `path` field.
  This only affects `nix-build --json` when "building" non-derivation things like fetched sources, which is a no-op.

- Introduce a new [`outputOf`](@docroot@/language/builtins.md#builtins-outputOf) builtin.
  It is part of the [`dynamic-derivations`](@docroot@/contributing/experimental-features.md#xp-feature-dynamic-derivations) experimental feature.

- The experimental nix command is now a `#!-interpreter` by appending the
  contents of any `#! nix` lines and the script's location to a single call.

  Verbatim strings may be passed in double backtick (```` `` ````) quotes.

  Some examples:
  ```
  #!/usr/bin/env nix
  #! nix shell --file ``<nixpkgs>`` hello --command bash

  hello | cowsay
  ```
  or with flakes:
  ```
  #!/usr/bin/env nix
  #! nix shell nixpkgs#bash nixpkgs#hello nixpkgs#cowsay --command bash

  hello | cowsay
  ```
  or
  ```bash
  #! /usr/bin/env nix
  #! nix shell --impure --expr ``
  #! nix with (import (builtins.getFlake "nixpkgs") {});
  #! nix terraform.withPlugins (plugins: [ plugins.openstack ])
  #! nix ``
  #! nix --command bash

  terraform "$@"
  ```
  or
  ```
  #!/usr/bin/env nix
  //! ```cargo
  //! [dependencies]
  //! time = "0.1.25"
  //! ```
  /*
  #!nix shell nixpkgs#rustc nixpkgs#rust-script nixpkgs#cargo --command rust-script
  */
  fn main() {
      for argument in std::env::args().skip(1) {
          println!("{}", argument);
      };
      println!("{}", std::env::var("HOME").expect(""));
      println!("{}", time::now().rfc822z());
  }
  // vim: ft=rust
  ```
