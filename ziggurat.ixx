
export module ziggurat;

import <iostream>;
import <random>;

#if defined(__GNUC__)
# define ZIGGURAT_LIKELY(x) __builtin_expect((x), 1)
# define ZIGGURAT_NOINLINE __attribute__((noinline))
#else
# define ZIGGURAT_LIKELY(x) (x)
# define ZIGGURAT_NOINLINE
#endif

export namespace cxx
{
	namespace ziggurat_detail
	{
		// is_pow2m1 checks if num + 1 is a power of two.
		template<typename T>
		inline constexpr bool is_pow2m1(T num)
		{
			return (num & (num + 1)) == 0;
		}

		// log2 computes the base-2 logarithm of num truncated to integer.
		inline constexpr std::size_t log2(std::uint64_t num)
		{
			return num / 2 ? 1 + log2(num / 2) : 0;
		}

		// generate_bits draws N random bits from given random number generator.
		template<std::size_t N, typename URNG>
		inline std::uint64_t generate_bits(URNG& random)
		{
			constexpr std::uint64_t mask = (std::uint64_t(1) << N) - 1;

			if (URNG::min() == 0 && URNG::max() >= mask && is_pow2m1(URNG::max())) {
				return std::uint64_t(random()) & mask;
			}
			else {
				std::uniform_int_distribution<std::uint64_t> dist(0, mask);
				return dist(random);
			}
		}

		// canonicalize transforms N bits into a floating-point number in [0, 1).
		template<std::size_t N, typename T>
		inline T canonicalize(std::uint64_t bits)
		{
			constexpr int real_bits = std::numeric_limits<T>::digits;
			constexpr int uint_bits = N;
			constexpr int data_bits = (real_bits < uint_bits ? real_bits : uint_bits);
			constexpr T norm = 1 / T(std::int64_t(1) << data_bits);
			return norm * T(bits >> (uint_bits - data_bits));
		}

		// gaussian returns exp(-x^2/2).
		template<typename T>
		inline T gaussian(T x)
		{
			return std::exp(T(-0.5) * x * x);
		}

		// normal_ziggurat holds a pre-computed ziggurat table.
		template<typename T>
		struct normal_ziggurat
		{
			static T const edges[0x81];
		};
	}

	// ziggurat_normal_distribution generates normal random numbers using the fast
	// ziggurat algorithm.
	template<typename T>
	class ziggurat_normal_distribution
	{
		// Pull in the ziggurat table to use.
		using ziggurat = ziggurat_detail::normal_ziggurat<T>;

	public:
		// result_type is an alias of T.
		using result_type = T;

		// param_type holds distribution parameters.
		struct param_type
		{
			using distribution_type = ziggurat_normal_distribution;

			// Default constructor initializes mean to 0 and stddev to 1.
			param_type() = default;

			// Single-parameter constructor initializes mean and stddev to given
			// values.
			explicit param_type(result_type mean, result_type stddev = 1)
				: mean_{ mean }, stddev_{ stddev }
			{
			}

			// mean returns the mean parameter.
			inline result_type mean() const
			{
				return mean_;
			}

			// stddev returns the stddev parameter.
			inline result_type stddev() const
			{
				return stddev_;
			}

			// Equality comparison p1 == p2 returns true if and only if mean and
			// stddev parameters, respectively, are the same for p1 and p2.
			friend bool operator==(param_type const& p1, param_type const& p2)
			{
				return p1.mean_ == p2.mean_ && p1.stddev_ == p2.stddev_;
			}

			friend bool operator!=(param_type const& p1, param_type const& p2)
			{
				return !(p1 == p2);
			}

			// Stream output write mean and stddev to a stream.
			template<typename Char, typename Tr>
			friend std::basic_ostream<Char, Tr>& operator<<(
				std::basic_ostream<Char, Tr>& os,
				param_type const& param
				)
			{
				using sentry_type = typename std::basic_ostream<Char, Tr>::sentry;

				// TODO: need to normalize stream flags?

				if (sentry_type sentry{ os }) {
					Char const space = os.widen(' ');
					os << param.mean_ << space << param.stddev_;
				}

				return os;
			}

			// Stream input reads mean and stddev from a stream.
			template<typename Char, typename Tr>
			friend std::basic_istream<Char, Tr>& operator>>(
				std::basic_istream<Char, Tr>& is,
				param_type& param
				)
			{
				using sentry_type = typename std::basic_istream<Char, Tr>::sentry;

				// TODO: need to normalize stream flags?

				if (sentry_type sentry{ is }) {
					param_type tmp;
					if (is >> tmp.mean_ >> tmp.stddev_) {
						param = tmp;
					}
				}

				return is;
			}

		private:
			result_type mean_ = 0;
			result_type stddev_ = 1;
		};

		// Default constructor creates a normal distribution with mean = 0 and
		// stddev = 1.
		ziggurat_normal_distribution() = default;

		// This constructor creates a normal distribution with given mean and
		// stddev.
		explicit ziggurat_normal_distribution(result_type mean, result_type stddev = 1)
			: param_{ mean, stddev }
		{
		}

		// This constructor creates a normal distribution having given
		// parameters.
		explicit ziggurat_normal_distribution(param_type const& param)
			: param_{ param }
		{
		}

		// reset does nothing; this is a RandomNumberDistribution requirement.
		void reset()
		{
		}

		// Invoking a distribution with a random number engine returns a newly
		// generated normal random number with the preconfigured parameters.
		template<typename URNG>
		inline T operator()(URNG& random)
		{
			return param_.mean() + param_.stddev() * sample(random);
		}

		// Invoking a distribution with a random number engine and a parameter
		// object returns a newly generated normal random number with given
		// parameters.
		template<typename URNG>
		inline T operator()(URNG& random, param_type const& param)
		{
			return param.mean() + param.stddev() * sample(random);
		}

		// mean returns the mean parameter of this distribution.
		result_type mean() const
		{
			return param_.mean();
		}

		// stddev returns the stddev parameter of this distribution.
		result_type stddev() const
		{
			return param_.stddev();
		}

		// param returns the parameters of this distribution as a param_type.
		param_type param() const
		{
			return param_;
		}

		// param sets the parameters of this distribution.
		void param(param_type const& param)
		{
			param_ = param;
		}

		// min returns -infinity.
		result_type min() const
		{
			return -std::numeric_limits<result_type>::infinity();
		}

		// max returns +infinity.
		result_type max() const
		{
			return std::numeric_limits<result_type>::infinity();
		}

	private:
		// sample generates a standard normal number.
		template<typename URNG>
		inline T sample(URNG& random) const
		{
			constexpr std::size_t bit_count = ziggurat_detail::log2(URNG::max() - URNG::min());

			for (;;)
			{
				auto const bits = ziggurat_detail::generate_bits<bit_count>(random);
				auto const uniform = ziggurat_detail::canonicalize<bit_count, T>(bits);
				auto const layer = std::size_t(bits & 0x7F);
				auto const sign = T((bits & 0x80) ? 1 : -1);

				auto const lower_edge = ziggurat::edges[layer];
				auto const upper_edge = ziggurat::edges[layer + 1];

				auto const x = uniform * lower_edge;

				if (ZIGGURAT_LIKELY(x < upper_edge)) {
					return sign * x;
				}

				if (layer == 0) {
					return sign * sample_from_tail(random);
				}

				if (check_accept(random, lower_edge, upper_edge, x)) {
					return sign * x;
				}
			}
		}

		template<typename URNG>
		ZIGGURAT_NOINLINE
			T sample_from_tail(URNG& random) const
		{
			T const tail_edge = ziggurat::edges[1];

			std::uniform_real_distribution<T> uniform;

			T x{}, y{};
			do {
				x = -std::log(uniform(random)) / tail_edge;
				y = -std::log(uniform(random));
			} while (2 * y < x * x);

			return tail_edge + x;
		}

		template<typename URNG>
		ZIGGURAT_NOINLINE
			bool check_accept(URNG& random, T lower_edge, T upper_edge, T x) const
		{
			// Rejection sampling from the interval [upper_edge, lower_edge].
			std::uniform_real_distribution<T> uniform(
				ziggurat_detail::gaussian(lower_edge),
				ziggurat_detail::gaussian(upper_edge)
			);
			return uniform(random) < ziggurat_detail::gaussian(x);
		}

	private:
		param_type param_;
	};

	// Equality comparison d1 == d2 compares the equality of distribution
	// parameters.
	template<typename T>
	bool operator==(
		ziggurat_normal_distribution<T> const& d1,
		ziggurat_normal_distribution<T> const& d2
		)
	{
		return d1.param() == d2.param();
	}

	template<typename T>
	bool operator!=(
		ziggurat_normal_distribution<T> const& d1,
		ziggurat_normal_distribution<T> const& d2
		)
	{
		return !(d1 == d2);
	}

	// Stream output operator writes mean and stddev parameters to a stream.
	template<typename Char, typename Tr, typename T>
	std::basic_ostream<Char, Tr>& operator<<(
		std::basic_ostream<Char, Tr>& os,
		ziggurat_normal_distribution<T> const& dist
		)
	{
		return os << dist.param();
	}

	// Stream input operator reads mean and stddev parameters from a stream.
	template<typename Char, typename Tr, typename T>
	std::basic_istream<Char, Tr>& operator>>(
		std::basic_istream<Char, Tr>& is,
		ziggurat_normal_distribution<T>& dist
		)
	{
		typename ziggurat_normal_distribution<T>::param_type param;
		if (is >> param) {
			dist.param(param);
		}
		return is;
	}

	// Pre-computed ziggurat table.
	template<typename T>
	T const ziggurat_detail::normal_ziggurat<T>::edges[] = {
			T(3.71308624674036292), T(3.44261985589665231), T(3.22308498457861869), T(3.083228858214214),
			T(2.97869625264501714), T(2.89434400701867078), T(2.82312535054596658), T(2.76116937238415394),
			T(2.70611357311872291), T(2.65640641125819288), T(2.61097224842861353), T(2.56903362592163953),
			T(2.53000967238546703), T(2.49345452209195129), T(2.4590181774083506), T(2.42642064553021219),
			T(2.395434278007468), T(2.36587137011398818), T(2.3375752413355313), T(2.31041368369500244),
			T(2.28427405967365704), T(2.25905957386533007), T(2.23468639558705728), T(2.21108140887472837),
			T(2.18818043207202084), T(2.16592679374484121), T(2.14427018235626177), T(2.1231657086697906),
			T(2.10257313518499966), T(2.08245623798772517), T(2.0627822745039639), T(2.04352153665067027),
			T(2.02464697337293442), T(2.00613386995896725), T(1.98795957412306135), T(1.97010326084971399),
			T(1.9525457295488895), T(1.93526922829190084), T(1.91825730085973256), T(1.90149465310031829),
			T(1.88496703570286983), T(1.86866114098954261), T(1.85256451172308778), T(1.83666546025338473),
			T(1.82095299659100562), T(1.80541676421404929), T(1.79004698259461947), T(1.77483439558076972),
			T(1.7597702248942324), T(1.74484612810837714), T(1.73005416055824424), T(1.71538674070811714),
			T(1.70083661856430157), T(1.68639684677348689), T(1.67206075409185284), T(1.65782192094820813),
			T(1.64367415685698326), T(1.62961147946467899), T(1.61562809503713356), T(1.6017183802152779),
			T(1.5878768648844015), T(1.57409821601675048), T(1.56037722235984133), T(1.54670877985350419),
			T(1.53308787766755672), T(1.51950958475937159), T(1.50596903685655104), T(1.49246142377461632),
			T(1.47898197698309875), T(1.46552595733579549), T(1.45208864288221728), T(1.43866531667746189),
			T(1.4252512545068623), T(1.41184171243976109), T(1.39843191412360701), T(1.38501703772514939),
			T(1.3715922024197329), T(1.35815245432242371), T(1.3446927517457139), T(1.33120794965767741),
			T(1.31769278320134386), T(1.30414185012042227), T(1.29054959191787399), T(1.27691027355170061),
			T(1.26321796144602927), T(1.24946649956433475), T(1.23564948325448198), T(1.22176023053096339),
			T(1.20779175040675857), T(1.19373670782377306), T(1.17958738465446178), T(1.16533563615504776),
			T(1.1509728421389771), T(1.1364898520030764), T(1.12187692257225491), T(1.1071236475235362),
			T(1.09221887689655461), T(1.07715062488193869), T(1.06190596368362034), T(1.0464709007525812),
			T(1.03083023605645652), T(1.01496739523930057), T(0.998864233480644681), T(0.982500803502761477),
			T(0.965855079388131865), T(0.948902625497913155), T(0.931616196601354973), T(0.913965251008802881),
			T(0.895915352566239664), T(0.877427429097716982), T(0.858456843178052043), T(0.838952214281208697),
			T(0.818853906683319033), T(0.798092060626276134), T(0.776583987876149906), T(0.754230664434511699),
			T(0.730911910621882877), T(0.706479611313609812), T(0.680747918645906114), T(0.653478638715044413),
			T(0.62435859730909038), T(0.592962942441980445), T(0.558692178375520654), T(0.520656038725148096),
			T(0.477437837253791464), T(0.426547986303309479), T(0.362871431028424229), T(0.272320864704672982),
			T(8.56006539842194211e-08)
	};
}

#undef ZIGGURAT_LIKELY
#undef ZIGGURAT_NOINLINE