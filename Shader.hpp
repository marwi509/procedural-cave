#include <vector>
#include <string>

/** Klass som skapar och håller koll på en shader, inte ens nära färdig men
 * 	förenklar saker lite grann */

using namespace std;

class Shader
{
	public:
	bool use();
	void stopUsing();
	void Delete();
	bool createShader(const char* vertexSource, const char* fragmentShader);
	~Shader();
	bool createUniformFloat(const char* name, float value);
	Shader(){compiled=false;}
	bool updateUniformFloat(const char* name, float value);
	GLuint getProgram();
	private:
	
	vector<GLint> locations;
	vector<string> names;
	
	GLuint program;
	bool compiled;
	void printLog(GLuint object);
	char* readFile(const char* filename, int& size);
	
	
};
