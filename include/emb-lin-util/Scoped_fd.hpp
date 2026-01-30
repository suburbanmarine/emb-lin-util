#pragma once

#include <memory>

class Scoped_fd
{
public:
	Scoped_fd(int fd);
	~Scoped_fd();

	Scoped_fd(const Scoped_fd& other) = delete;
	Scoped_fd& operator=(const Scoped_fd& other) = delete;
protected:
	int m_fd;
};