{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "pentelka-dev-shell";

  buildInputs = with pkgs; [
    # Toolchain
    gcc
    gdb
    cmake
    ninja

    # Qt6 (Widgets + QML)
    qt6.qtbase
    qt6.qtdeclarative
    qt6.qttools

    # Optional but useful
    pkg-config
  ];

  shellHook = ''
    echo "Pentelka development shell"
    echo "Qt6 + CMake + Ninja ready"

    # Qt environment setup
    export QT_PLUGIN_PATH=${pkgs.qt6.qtbase}/lib/qt-6/plugins
    export QML2_IMPORT_PATH=${pkgs.qt6.qtdeclarative}/lib/qt-6/qml

    # Helps CMake find Qt
    export CMAKE_PREFIX_PATH=${pkgs.qt6.qtbase}
  '';
}

