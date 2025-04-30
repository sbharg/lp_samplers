# $\ell_p$ Samplers

A C++ implementation of low-memory probabilistic data structures for $\ell_p$ sampling for $p \in \{1, 2\}$ in data streams.

## Building
This project uses CMake for building. To build the library and test executables:

```bash
mkdir build
cd build
cmake ..
make
```

## References

- Moses Charikar, Kevin Chen, and Martin Farach-Colton. Finding frequent items in data streams. *Theoretical Computer Science*, 312(1):3–15, 2004. [doi:10.1016/S0304-3975(03)00400-6](https://www.doi.org/10.1016/S0304-3975(03)00400-6).

- Graham Cormode and Hossein Jowhari. $L_p$ samplers and their applications: A survey. *ACM Computing
Surveys*, 52(1):16:1–16:31, 2019. [doi:10.1145/3297715](https://www.doi.org/10.1145/3297715).

- Hossein Jowhari, Mert Sağlam, and Gábor Tardos. Tight bounds for $L_p$ samplers, finding duplicates in streams, and related problems. In *Proc. of the 30th Symposium on Principles of Database Systems (PODS 2011)*, pages 49–58, 2011. [doi:10.1145/1989284.1989289](https://www.doi.org/10.1145/1989284.1989289).

- Daniel M. Kane, Jelani Nelson, and David P. Woodruff. On the exact space complexity of sketching and
streaming small norms. In *Proc. of the 21st ACM-SIAM Symposium on Discrete Algorithms (SODA 2010)*,
pages 1161–1178. SIAM, 2010. [doi:10.1137/1.9781611973075.93](https://www.doi.org/10.1137/1.9781611973075.93).

- Mikkel Thorup and Yin Zhang. Tabulation-based 5-independent hashing with applications to linear probing and second moment estimation. *SIAM Journal on Computing*, 41(2):293–331, 2012. [doi:10.1137/100800774](https://www.doi.org/10.1137/100800774).
