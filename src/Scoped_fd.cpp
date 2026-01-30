#include <emb-lin-util/Scoped_fd.hpp>


Scoped_fd::Scoped_fd(int fd)
{
	m_fd = std::unique_ptr<int>()
}

Scoped_fd::~Scoped_fd()
{
	if(fd >= 0)
	{
		close(m_fd);
		m_fd = -1;
	}
}
