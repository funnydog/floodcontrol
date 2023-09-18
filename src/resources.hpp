#pragma once

enum class ViewID
{
	None,
	Title,
	GamePlay,
	GameOver,
};

enum class FontID
{
	Pericles36,
};

enum class TextureID
{
	Background,
	TileSheet,
	TitleScreen,
};

template<typename Resource, typename Identifier>
class ResourceHolder;

class Font;
typedef ResourceHolder<Font, FontID> FontHolder;

class Texture;
typedef ResourceHolder<Texture, TextureID> TextureHolder;
