#pragma once

enum class ViewID
{
	None,
	Title,
	GamePlay,
};

enum class FontID
{
	Title,
	Subtitle,
	Body,
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
