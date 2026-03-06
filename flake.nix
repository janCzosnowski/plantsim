{
  description = "A flake for Plantsim made by AlikornSause";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    rlImGui={
      url = "github:raylib-extras/rlImGui";
      flake = false;
    };
    ImGui={
      url = "github:ocornut/imgui";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, flake-utils, rlImGui, ImGui}:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        ffmpegMinimal= pkgs.ffmpeg.override {
          withDocumentation = false;
          withManPages = false;
          withNetwork = false;
          withStatic = true;
          withPic = true;
          withShared = false;
          buildFfmpeg = false;
          buildFfplay = false;
          buildFfprobe = false;
          buildAvcodec = true;
          buildAvformat = true;
          buildAvutil = true;
          buildSwscale = true;
          buildSwresample = true;
          withOpenh264 = true;
          withVulkan = false;
        };


        raylibStatic = pkgs.raylib.overrideAttrs (old: {
          cmakeFlags = old.cmakeFlags ++ [
            "-DBUILD_SHARED_LIBS=OFF"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
            "-DBUILD_EXAMPLES=OFF"
            "-DBUILD_TESTING=OFF"
          ];
        });
      in
      {

        packages.default = pkgs.stdenv.mkDerivation {
          pname = "plantsim";
          version = "1.0";
          description = "Cellular automata simulation modeling plant genomes, cell reproduction, mutation, and evolution.";

          src = ./.;

          nativeBuildInputs = [
            pkgs.pkg-config
          ];

          buildInputs = [
            raylibStatic
            ffmpegMinimal
            pkgs.glfw
            pkgs.glew
            pkgs.xz
            pkgs.zlib
            pkgs.libdrm
            pkgs.mesa
            pkgs.libX11
            pkgs.bzip2
            pkgs.lame
            pkgs.xvidcore
            pkgs.libtheora
            pkgs.openapv
            pkgs.soxr
            pkgs.libvdpau
          ];

          configurePhase = ''
              mkdir -p ./libs/
              ln -s ${rlImGui} ./libs/rlImGui
              ln -s ${ImGui} ./libs/imgui
            '';

          buildPhase = ''
            pwd
           g++ -std=c++20 \
            src/*.cpp \
            libs/imgui/*.cpp \
            libs/rlImGui/rlImGui.cpp \
            -Ilibs/imgui -Ilibs/rlImGui -Isrc \
            $(pkg-config --cflags raylib libavformat libavcodec libswscale libavutil glew) \
            -o plantsim \
            $(pkg-config --libs raylib libavformat libavcodec libswscale libavutil glew) \
            -lglfw -lGL -lX11 -lpthread -ldl -lrt -lz -llzma -ldrm
           '';

          installPhase = ''
            mkdir -p $out/bin
            cp plantsim $out/bin/
          '';
        };

      });
}
