#include "HelloWorld.hpp"
#include <algorithm>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

// Note: Allman brace style used throughout; deviates from the K&R style shown
// in the C++ Core Guidelines examples (NL.17) but applied consistently.

namespace
{

// -----------------------------------------------------------------------
// Student — plain aggregate; follows the Rule of 0 because all members
// manage their own resources (std::string handles its own storage).
// -----------------------------------------------------------------------
struct Student
{
	int32_t		m_id	= 0;
	std::string	m_name	= {};
	double		m_score	= 0.0;
};

// -----------------------------------------------------------------------
// Grade_Book
// -----------------------------------------------------------------------
class Grade_Book
{
public:
	// Rule of 6: all six special member functions explicitly defined.
	Grade_Book() = default;
	~Grade_Book() = default;
	Grade_Book(const Grade_Book& in_other) = default;
	Grade_Book&	operator=(const Grade_Book& in_other) = default;
	Grade_Book(Grade_Book&& in_other) = default;
	Grade_Book&	operator=(Grade_Book&& in_other) = default;

	// F.18: take name by value and move it to avoid an unnecessary copy
	void	add_student(int32_t in_id, std::string in_name, double in_score)
	{
		m_students.push_back({in_id, std::move(in_name), in_score});
		if (m_on_student_added)
		{
			m_on_student_added(m_students.back());
		}
	}

	// Accepts any callable matching the signature — stored for later use
	void	set_on_student_added(std::function<void(const Student&)> in_callback)
	{
		m_on_student_added = std::move(in_callback);
	}

	[[nodiscard]] std::optional<Student>	find_student(int32_t in_id) const
	{
		auto student_iterator = std::find_if(m_students.begin(), m_students.end(),
			[in_id](const Student& in_student) { return in_student.m_id == in_id; });

		if (student_iterator == m_students.end())
		{
			return std::nullopt;
		}

		return *student_iterator;
	}

	[[nodiscard]] double	average_score() const
	{
		if (m_students.empty())
		{
			return 0.0;
		}

		double total = std::accumulate(m_students.begin(), m_students.end(), 0.0,
			[](double in_sum, const Student& in_student) { return in_sum + in_student.m_score; });

		return total / static_cast<double>(m_students.size());
	}

	void	for_each(std::function<void(const Student&)> in_visitor) const
	{
		for (const auto& student : m_students)
		{
			in_visitor(student);
		}
	}

	[[nodiscard]] std::vector<Student>	filter(std::function<bool(const Student&)> in_predicate) const
	{
		std::vector<Student> result;
		std::copy_if(m_students.begin(), m_students.end(), std::back_inserter(result), in_predicate);
		return result;
	}

	[[nodiscard]] std::vector<Student>	sorted_by(std::function<bool(const Student&, const Student&)> in_compare) const
	{
		std::vector<Student> result = m_students;
		std::sort(result.begin(), result.end(), in_compare);
		return result;
	}

private:
	std::vector<Student>					m_students;
	std::function<void(const Student&)>	m_on_student_added;
};

// Template function — prints any vector whose elements expose m_id, m_name, m_score.
// A concept (C++20) could constrain T here; omitted to keep the example focused.
template<typename T>
void	print_collection(const std::vector<T>& in_items)
{
	for (const auto& item : in_items)
	{
		std::cout << std::format("  [{:3d}]  {:<20s}  {:5.1f}\n", item.m_id, item.m_name, item.m_score);
	}
}

} // namespace

// -----------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------
void	run_demo_cpp(void)
{
	std::cout << "\n=== C++ Demo ===\n\n";

	Grade_Book book;

	// Stored closure: fires each time a student is added
	book.set_on_student_added([](const Student& in_student)
	{
		std::cout << std::format("  + Added: {}\n", in_student.m_name);
	});

	book.add_student(1, "Alice",	92.5);
	book.add_student(2, "Bob",		78.0);
	book.add_student(3, "Carol",	85.5);
	book.add_student(4, "Dave",		61.0);
	book.add_student(5, "Eve",		97.0);

	std::cout << "\nAll students:\n";
	book.for_each([](const Student& in_student)
	{
		std::cout << std::format("  [{:3d}]  {:<20s}  {:5.1f}\n", in_student.m_id, in_student.m_name, in_student.m_score);
	});

	std::cout << std::format("\nAverage: {:.1f}\n", book.average_score());

	// Lambda capturing by value — safe because the lambda does not outlive threshold
	double threshold = 80.0;
	auto high_achievers = book.filter([threshold](const Student& in_student)
	{
		return in_student.m_score >= threshold;
	});
	std::cout << std::format("\nStudents scoring >= {:.0f}:\n", threshold);
	print_collection(high_achievers);

	// Comparator lambda passed to sorted_by
	auto by_score_desc = book.sorted_by([](const Student& in_lhs, const Student& in_rhs)
	{
		return in_lhs.m_score > in_rhs.m_score;
	});
	std::cout << "\nRanked by score:\n";
	print_collection(by_score_desc);

	// Mutable lambda: captures a counter by value and mutates it across calls.
	// Without 'mutable', modifying a captured-by-value variable is a compile error.
	int32_t rank = 0;
	auto print_ranked = [rank](const Student& in_student) mutable
	{
		std::cout << std::format("  #{}: {}\n", ++rank, in_student.m_name);
	};
	std::cout << "\nRanks (via mutable lambda):\n";
	for (const auto& student : by_score_desc)
	{
		print_ranked(student);
	}

	// Immediately invoked lambda: computes a one-off value inline without
	// needing a named variable or a helper function
	double top_score = [&book]()
	{
		double max_score = 0.0;
		book.for_each([&max_score](const Student& in_student)
		{
			if (in_student.m_score > max_score)
			{
				max_score = in_student.m_score;
			}
		});
		return max_score;
	}();
	std::cout << std::format("\nTop score: {:.1f}\n", top_score);

	// std::optional: structured binding on the result of find_student
	if (auto found = book.find_student(3))
	{
		std::cout << std::format("\nFound student 3: {}\n", found->m_name);
	}

	if (!book.find_student(99))
	{
		std::cout << "Student 99: not found\n";
	}
}

// -----------------------------------------------------------------------
// Hello
// -----------------------------------------------------------------------
void	hello_cpp(void)
{
	std::cout << "Hello from C++" << std::endl;
}
