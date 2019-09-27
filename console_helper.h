#pragma once
#include <conio.h>
#include <iostream>
#include <Windows.h>

constexpr int k_buffer_width{ 5 };
constexpr int k_buffer_height{ 3 };

constexpr int k_buffer_max_width{ 90 };
constexpr int k_buffer_max_height{ 26 };

constexpr int k_buffer_size{ (k_buffer_width + 1) * k_buffer_height };
constexpr int k_buffer_max_size{ (k_buffer_max_width + 1) * k_buffer_max_height };

struct s_pos
{
	s_pos() {};
	s_pos(int _x, int _y) : x{ _x }, y{ _y } {};

	int x{};
	int y{};

	bool operator==(const s_pos& other)
	{
		if ((x == other.x) && (y == other.y))
		{
			return true;
		}

		return false;
	}

	void move_up()
	{
		if (y > 0)
		{
			--y;
		}
	}

	void move_down(int max_y)
	{
		if (y < max_y)
		{
			++y;
		}
	}

	void move_left()
	{
		if (x > 0)
		{
			--x;
		}
	}

	void move_right(int max_x)
	{
		if (x < max_x)
		{
			++x;
		}
	}
};

enum class e_key
{
	none,

	esc,
	up,
	down,
	left,
	right,
};

class console_helper
{
public:
	console_helper(unsigned int buffer_x_base, unsigned int buffer_y_base, unsigned int buffer_width, unsigned int buffer_height) :
		m_buffer_x_base{ buffer_x_base }, m_buffer_y_base{ buffer_y_base }
	{
		buffer_width = min(buffer_width, k_buffer_max_width);
		buffer_height = min(buffer_height, k_buffer_max_height);

		m_buffer_width = buffer_width;
		m_buffer_height = buffer_height;
		m_buffer_size = (m_buffer_width + 1) * m_buffer_height;

		CONSOLE_CURSOR_INFO info{};
		info.bVisible = false;
		info.dwSize = sizeof(CONSOLE_CURSOR_INFO);

		SetConsoleCursorInfo(m_h_console, &info);
	};
	~console_helper() {};

public:

	void clear_screen()
	{
		for (int y = 0; y < m_buffer_height; ++y)
		{
			memset(m_buffer[y], '%', sizeof(char) * m_buffer_width);
			m_buffer[y][m_buffer_width] = '\0';
		}
	}

	void set_text(const s_pos& pos, const char* text)
	{
		set_text(pos.x, pos.y, text);
	}

	void set_text(int x, int y, const char* text)
	{
		x = max(x, 0);
		y = max(y, 0);
		x = min(x, m_buffer_width - 1);
		y = min(y, m_buffer_height - 1);

		int len = strlen(text);

		for (int i = 0; i < len; ++i)
		{
			if (x + i >= m_buffer_width)
			{
				break;
			}

			set_character(x + i, y, text[i]);
		}
	}

	void set_character(const s_pos& pos, char ch)
	{
		set_character(pos.x, pos.y, ch);
	}

	void set_character(int x, int y, char ch)
	{
		x = max(x, 0);
		y = max(y, 0);
		x = min(x, m_buffer_width - 1);
		y = min(y, m_buffer_height - 1);

		m_buffer[y][x] = ch;
	}

	void print_screen()
	{
		//빌트인 자료형 처럼 작은 자료형이 아닌 클레스 같은 큰 자료형을 선언이 반복문 안에 있을때, static을 앞에다가 붙여주면 
		// 처음 한번만 선언되고, 프로그램이 종료시까지 살아있다. life time은 전역변수와 같지만 scope는 한정적이다.
		static COORD coord{};
		
		coord.X = m_buffer_x_base;
		for (int i = 0; i < m_buffer_height; ++i)
		{
			coord.Y = m_buffer_y_base + i;

			SetConsoleCursorPosition(m_h_console, coord);
			std::cout << m_buffer[i];
		}
	}

	void print_border(char border_letter = '#')
	{
		static COORD coord{};
				
		// 위 테두리
		coord.Y = m_buffer_y_base - 1;
		if (coord.Y >= 0)
		{
			for (int i = 0; i < m_buffer_width + 2; ++i)
			{
				coord.X = m_buffer_x_base + i - 1;
				if (coord.X >= 0)
				{
					SetConsoleCursorPosition(m_h_console, coord);

					std::cout << border_letter;
				}
			}
		}

		// 아래 테두리
		coord.Y = m_buffer_y_base + m_buffer_height;
		for (int i = 0; i < m_buffer_width + 2; ++i)
		{
			coord.X = m_buffer_x_base + i - 1;
			if (coord.X >= 0)
			{
				SetConsoleCursorPosition(m_h_console, coord);
				std::cout << border_letter;
			}
		}
		

		// 왼쪽 테두리
		coord.X = m_buffer_x_base - 1;
		if (coord.X >= 0)
		{
			for (int i = 0; i < m_buffer_height; ++i)
			{
				coord.Y = m_buffer_y_base + i;
				if (coord.Y >= 0)
				{
					SetConsoleCursorPosition(m_h_console, coord);
					std::cout << border_letter;
				}
			}
		}

		// 오른쪽 테두리
		coord.X = m_buffer_x_base + m_buffer_width;
		for (int i = 0; i < m_buffer_height; ++i)
		{
			coord.Y = m_buffer_y_base + i;
			if (coord.Y >= 0)
			{
				SetConsoleCursorPosition(m_h_console, coord);
				std::cout << border_letter;
			}
		}
	}

	void set_cursor_at(int x, int y)
	{
		if (y < m_buffer_height)
		{
			x = max(x, m_buffer_width + m_buffer_x_base + 1);
		}

		static COORD coord{};
		coord.X = x;
		coord.Y = y;
		SetConsoleCursorPosition(m_h_console, coord);
	}

	e_key detect_key()
	{
		if (_kbhit())
		{
			int key = _getch();
			if (key == 27)
			{
				return e_key::esc;
			}
			else if (key == 224)
			{
				key = _getch();
				if (key == 72)
				{
					return e_key::up;
				}
				else if (key == 80)
				{
					return e_key::down;
				}
				else if (key == 75)
				{
					return e_key::left;
				}
				else if (key == 77)
				{
					return e_key::right;
				}
			}
		}

		return e_key::none;
	}

	int get_buffer_width()
	{
		return m_buffer_width;
	}

	int get_buffer_height()
	{
		return m_buffer_height;
	}

private:
	HANDLE m_h_console{ GetStdHandle(STD_OUTPUT_HANDLE) };

	char m_buffer[k_buffer_max_height][k_buffer_max_width]{};

	int m_buffer_width{ k_buffer_width };
	int m_buffer_height{ k_buffer_height };
	int m_buffer_size{ k_buffer_size };

	unsigned int m_buffer_x_base{};
	unsigned int m_buffer_y_base{};
};