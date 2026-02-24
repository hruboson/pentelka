{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "pentelka-dev-shell";

  buildInputs = with pkgs; [
	gcc
	gdb
	cmake
	ninja
	qtcreator
	qt6.qtbase
	qt6.qtdeclarative
	qt6.qttools

	pkg-config
  ];

  nativeBuildInputs = with pkgs; [
	cmake
	ninja
	pkg-config
	qt6.wrapQtAppsHook 
  ];

  shellHook = ''
    echo "Pentelka development shell"
    echo "Qt6 + CMake + Ninja ready"

    # Qt environment setup
    export QT_PLUGIN_PATH=${pkgs.qt6.qtbase}/lib/qt-6/plugins
    export QML2_IMPORT_PATH=${pkgs.qt6.qtdeclarative}/lib/qt-6/qml

    # Helps CMake find Qt
    export CMAKE_PREFIX_PATH="${pkgs.qt6.qtbase}:${pkgs.qt6.qtdeclarative}:$CMAKE_PREFIX_PATH"
  '';
}

