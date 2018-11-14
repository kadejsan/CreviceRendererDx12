#include "stdafx.h"
#include <stb_image.h>
#include "image.h"

Image::Image()
	: m_width(0)
	, m_height(0)
	, m_channels(0)
	, m_hdr(false)
{}

std::shared_ptr<Image> Image::FromFile(const std::string& filename, int channels)
{
	std::shared_ptr<Image> image{ new Image };

	if (stbi_is_hdr(filename.c_str())) 
	{
		float* pixels = stbi_loadf(filename.c_str(), &image->m_width, &image->m_height, &image->m_channels, channels);
		if (pixels) 
		{
			image->m_pixels.reset(reinterpret_cast<unsigned char*>(pixels));
			image->m_hdr = true;
		}
	}
	else 
	{
		unsigned char* pixels = stbi_load(filename.c_str(), &image->m_width, &image->m_height, &image->m_channels, channels);
		if (pixels) 
		{
			image->m_pixels.reset(pixels);
			image->m_hdr = false;
		}
	}
	if (channels > 0) 
	{
		image->m_channels = channels;
	}

	if (!image->m_pixels) 
	{
		return nullptr;
	}
	return image;
}
