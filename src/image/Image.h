#pragma once

#include <string_view>
#include <cstdint>

class Image
{
public:
    Image();

    Image(int w, int h, int c);

    Image(const Image &other);

    Image(Image &&other) noexcept;

    Image &operator=(const Image &other);

    Image &operator=(Image &&other) noexcept;

    ~Image();

    void clear();

    void load(std::string_view filename);

    void save(std::string_view filename) const;

    void resize(int new_width, int new_height);

    Image calculateEdges() const;

    uint8_t *data() const;

    int width() const noexcept;

    int height() const noexcept;

    int channels() const noexcept;

    uint8_t *operator()(int x, int y);
    const uint8_t *operator()(int x, int y) const;

    uint8_t *operator[](int index);
    const uint8_t *operator[](int index) const;

private:
    int width_, height_, channels_;
    uint8_t *data_ = nullptr;
};
