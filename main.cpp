#include <Windows.h>

#include "System/Game.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Game game;
	game.Initialize();
	game.Run();
	return 0;
}
