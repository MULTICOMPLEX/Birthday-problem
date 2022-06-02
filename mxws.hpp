
#include <numeric>
#include <random>

template <typename RN>
class mxws
{

private:

	std::random_device r;

public:

	uint64_t x, w, x1, x2, w1, w2;

	typedef RN result_type;

	void seed()
	{
		init();
		x1 = x2 = 1;
	}

	mxws(const std::seed_seq& seq)
	{
		if (seq.size() == 2)
		{
			std::vector<uint32_t> seeds(seq.size());
			seq.param(seeds.rbegin());
			w = (uint64_t(seeds[1]) << 32) | seeds[0];
			x = 1;

			w1 = w;
			w2 = w1 + 1;
			x1 = x2 = 1;
		}

		else init();
	}

	mxws()
	{
		init();
	}

	mxws(const uint64_t& seed)
	{
		init();
	}

	void init()
	{
		w = (uint64_t(r()) << 32) | r();
		x = 1;
		w1 = w;
		w2 = w1 + 1;
		x1 = x2 = 1;
	}

	void init(const uint64_t& seed)
	{
		w = seed;
		x = 1;
	}

	virtual ~mxws() = default;

	const RN min() { return std::numeric_limits<RN>::min(); }
	const RN max() { return std::numeric_limits<RN>::max(); }

	inline RN operator()()
		requires
	std::same_as<RN, uint32_t>
	{
		w += x = std::rotr(x *= w, 32);
		return RN(x);
	}

	inline RN operator()()
		requires
	std::same_as<RN, uint64_t>
	{
		x1 *= w1;
		x1 = std::rotr(x1, 32);
		w1 += x1;

		x2 *= w2;
		x2 = std::rotr(x2, 32);
		w2 += x2;

		return (x1 << 32) | uint32_t(x2);
	}

	template <typename T>
		requires std::floating_point<T>&&
	std::same_as<RN, uint64_t>
		inline T operator()(const T& f)
	{
		return ((*this)() >> 11) / T(9007199254740992) * f;
	}

	template <typename T>
		requires std::floating_point<T>&&
	std::same_as<RN, uint32_t>
		inline T operator()(const T& f)
	{
		return (*this)() / T(4294967296) * f;
	}

	template <typename T>
		requires std::floating_point<T>
	inline T operator()(const T& min, const T& max)
	{
		return (*this)(1.0) * (max - min) + min;
	}

	template <typename T, typename U>
		requires std::integral<T>&& std::floating_point<U>
	inline U operator()(const T& min, const U& max)
	{
		return (*this)(1.0) * (max - min) + min;
	}

	template <typename T>
		requires std::integral<T>
	inline T operator()(const T& max)
	{
		return (*this)() % (max + 1);
	}

	template <typename T>
		requires std::integral<T>
	inline T operator()(const T& min, const T& max)
	{
		return min + ((*this)() % (max - min + 1));
	}

	template <typename T>
		requires std::floating_point<T>
	inline int to_int(T min, T max)
	{
		min = to_int((*this)(1.0) * (max - min) + min);
		return min;
	}

	template <typename T>
		requires std::floating_point<T>
	inline int to_int(T x)
	{
		x = (*this)(x) + 6755399441055744.0;
		return reinterpret_cast<int&>(x);
	}

	template <typename R, typename I, typename L, typename B>
		requires
	std::same_as<R, double>&&
		std::integral<I>&&
		std::same_as<L, std::uint64_t>&&
		std::convertible_to<B, bool>
		std::tuple<R, I> inline Probability_Wave(const I& cycle_SIZE,
			std::vector<I>& cycle, const I& N_cycles, const L& TRIALS, const B& fast_algo) {

		R rng_range;

		I cycle_size;

		if (fast_algo && (cycle_SIZE > N_cycles / 2.)) {
			cycle_size = I(ceil(log(cycle_SIZE) * 2));
			rng_range = cycle_SIZE / sqrt(log2(cycle_SIZE));
		}

		else {
			cycle_size = cycle_SIZE;
			rng_range = sqrt(cycle_size) + log(cycle_size / 4);
		}

		R random_walk;

		for (L i = 0; i < TRIALS; i++)
		{
			random_walk = 0;

			for (I j = 0; j < cycle_size; j++)
				random_walk += (*this)(rng_range);

			cycle[I(random_walk) % cycle_SIZE]++;
		}

		return std::make_tuple(rng_range, cycle_size);
	}
};