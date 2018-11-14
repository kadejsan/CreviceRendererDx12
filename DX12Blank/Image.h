#pragma once

class Image
{
public:
	static std::shared_ptr<Image> FromFile(const std::string& filename, int channels = 4);

	int Width() const { return m_width; }
	int Height() const { return m_height; }
	int Channels() const { return m_channels; }
	int BytesPerPixel() const { return m_channels * (m_hdr ? sizeof(float) : sizeof(unsigned char)); }
	int Pitch() const { return m_width * BytesPerPixel(); }
	long DataSize() const { return (long)Pitch() * (long)m_height; }

	bool IsHDR() const { return m_hdr; }

	template<typename T>
	const T* Pixels() const
	{
		return reinterpret_cast<const T*>(m_pixels.get());
	}

private:
	Image();

	int								m_width;
	int								m_height;
	int								m_channels;
	bool							m_hdr;
	std::unique_ptr<unsigned char>	m_pixels;
};