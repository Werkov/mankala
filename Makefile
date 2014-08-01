all: toguz

%: %.cpp
	g++ -g --std=c++11 $< -o $@

clean:
	rm -f toguz
