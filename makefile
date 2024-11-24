
all:
	g++ -I src/include -L src/lib -o main project_150.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf