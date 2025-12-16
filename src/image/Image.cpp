#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include "include/stb/stb_image.h"
#include "include/stb/stb_image_write.h"
#include "include/stb/stb_image_resize.h"

#include <stdexcept>
#include <algorithm>
#include <cmath>

Image::Image() : width_(0), height_(0), channels_(3), data_(nullptr) {}

Image::Image(int w, int h, int c) : width_(w), height_(h), channels_(c), data_(nullptr)
{
    if (w <= 0 || h <= 0 || c <= 0)
        throw std::invalid_argument("Width, height, and channels must be positive values");

    size_t dataSize = w * h * c;

    data_ = static_cast<uint8_t *>(malloc(dataSize));
    if (!data_)
        throw std::runtime_error("Failed to allocate memory for image");

    std::fill(data_, data_ + dataSize, 255);
}

Image::Image(const Image &other) : width_(other.width_), height_(other.height_), channels_(other.channels_)
{
    size_t dataSize = width_ * height_ * channels_;
    data_ = static_cast<uint8_t *>(malloc(dataSize));
    if (!data_)
    {
        throw std::runtime_error("Failed to allocate memory for image copy");
    }
    std::copy_n(other.data_, dataSize, data_);
}

Image::Image(Image &&other) noexcept : width_(other.width_), height_(other.height_), channels_(other.channels_), data_(other.data_)
{
    data_ = other.data_;
    width_ = other.width_;
    height_ = other.height_;
    channels_ = other.channels_;

    other.data_ = nullptr;
    other.width_ = other.height_ = other.channels_ = 0;
}

Image &Image::operator=(const Image &other)
{
    if (this == &other)
        return *this;

    clear();

    width_ = other.width_;
    height_ = other.height_;
    channels_ = other.channels_;

    if (other.data_)
    {
        size_t dataSize = static_cast<size_t>(width_) * height_ * channels_;
        data_ = static_cast<uint8_t *>(malloc(dataSize));
        if (!data_)
        {
            throw std::runtime_error("Failed to allocate memory for image assignment");
        }
        std::copy_n(other.data_, dataSize, data_);
    }
    else
    {
        data_ = nullptr;
    }
    return *this;
}

Image &Image::operator=(Image &&other) noexcept
{
    if (this == &other)
        return *this;

    clear();

    data_ = other.data_;
    width_ = other.width_;
    height_ = other.height_;
    channels_ = other.channels_;

    other.data_ = nullptr;
    other.width_ = other.height_ = other.channels_ = 0;

    return *this;
}

Image::~Image()
{
    clear();
}

void Image::clear()
{
    if (data_)
    {
        stbi_image_free(data_);
        data_ = nullptr;
    }
    width_ = height_ = channels_ = 0;
}

void Image::load(std::string_view filename)
{
    clear();
    int desired_channels = 3;
    data_ = stbi_load(filename.data(), &width_, &height_, nullptr, desired_channels);
    if (!data_)
    {
        throw std::runtime_error("Failed to load image");
    }
    channels_ = desired_channels;
}

void Image::save(std::string_view filename) const
{
    if (!data_)
    {
        throw std::runtime_error("No image data to save");
    }

    stbi_write_png(filename.data(), width_, height_, channels_, data_, width_ * channels_);
}

void Image::resize(int new_width, int new_height)
{
    if (!data_)
    {
        throw std::runtime_error("No image data to resize");
    }
    size_t newSize = static_cast<size_t>(new_width) * new_height * channels_;
    uint8_t *resized_data = static_cast<uint8_t *>(malloc(newSize));
    if (!resized_data)
    {
        throw std::runtime_error("Failed to allocate memory for resized image");
    }

    int result = stbir_resize_uint8(data_, width_, height_, 0,
                                    resized_data, new_width, new_height, 0, channels_);

    if (result == 0)
    {
        free(resized_data);
        throw std::runtime_error("Failed to resize image");
    }

    stbi_image_free(data_);

    data_ = resized_data;
    width_ = new_width;
    height_ = new_height;
}

Image Image::calculateEdges() const
{
    Image edges(width_, height_, channels_);
    for (int y = 1; y < height_ - 1; ++y)
    {
        for (int x = 1; x < width_ - 1; ++x)
        {
            for (int c = 0; c < channels_; ++c)
            {
                int gx = (-1) * (*this)(x - 1, y - 1)[c] + 1 * (*this)(x + 1, y - 1)[c] +
                         (-2) * (*this)(x - 1, y)[c] + 2 * (*this)(x + 1, y)[c] +
                         (-1) * (*this)(x - 1, y + 1)[c] + 1 * (*this)(x + 1, y + 1)[c];

                int gy = 1 * (*this)(x - 1, y + 1)[c] + 2 * (*this)(x, y + 1)[c] + 1 * (*this)(x + 1, y + 1)[c] +
                         (-1) * (*this)(x - 1, y - 1)[c] - 2 * (*this)(x, y - 1)[c] - 1 * (*this)(x + 1, y - 1)[c];

                int grad = std::hypot(gx, gy);
                edges(x, y)[c] = ~(std::clamp<long long>(grad - 50, 0, 255));
            }
        }
    }

    return edges;
}

uint8_t *Image::data() const noexcept
{
    return data_;
}

int Image::width() const noexcept
{
    return width_;
}

int Image::height() const noexcept
{
    return height_;
}

int Image::channels() const noexcept
{
    return channels_;
}

uint8_t *Image::operator()(int x, int y) noexcept
{
    return data_ + (y * width_ + x) * channels_;
}

const uint8_t *Image::operator()(int x, int y) const noexcept
{
    return data_ + (y * width_ + x) * channels_;
}

uint8_t *Image::operator[](int index) noexcept
{
    return data_ + index;
}
const uint8_t *Image::operator[](int index) const noexcept
{
    return data_ + index;
}