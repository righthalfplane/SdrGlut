BMP files

ffmpeg -vcodec bmp -i /path/to/texture-file.bmp -vcodec rawvideo -f rawvideo -pix_fmt rgb565 texture.raw

PNG files

ffmpeg -vcodec png -i /path/to/texture-file.png -vcodec rawvideo -f rawvideo -pix_fmt rgb565 texture.raw

Loading a raw format picture as a texture in OpenGL

int fd = open("texture.raw", O_RDONLY);
read(fd, texture_buffer, raw_picture_file_size_in_bytes);
close(fd);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, picture_width, picture_height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texture_buffer);
glGenerateMipmap(GL_TEXTURE_2D);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
