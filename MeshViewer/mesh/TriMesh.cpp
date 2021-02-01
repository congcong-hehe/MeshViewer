#include "TriMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <QOpenGLWidget>

TriMesh::TriMesh(): vbo_(QOpenGLBuffer::VertexBuffer),
ebo_(QOpenGLBuffer::IndexBuffer)
{

}

TriMesh::~TriMesh()
{

}

void TriMesh::loadFile(std::string file_name)
{
	std::ifstream in(file_name);

	if (!in)
	{
		std::cout << "The file can not be opened!" << std::endl;
		return;
	}
	
	REAL x_min = FLT_MAX, x_max = -FLT_MAX;
	REAL y_min = FLT_MAX, y_max = -FLT_MAX;
	REAL z_min = FLT_MAX, z_max = -FLT_MAX;

	int n = 0;	// �������
	std::string line;
	while (std::getline(in, line))
	{
		std::string tag;
		std::istringstream is(line);
		
		is >> tag;
		if (tag == "v")
		{
			Vec3f vec3f;
			is >> vec3f.x >> vec3f.y >> vec3f.z;

			// �����Χ��
			x_min = fmin(x_min, vec3f.x);
			x_max = fmax(x_max, vec3f.x);
			y_min = fmin(y_min, vec3f.y);
			y_max = fmax(y_max, vec3f.y);
			z_min = fmin(z_min, vec3f.z);
			z_max = fmax(z_max, vec3f.z);

			vtxs_.push_back(vec3f);
			n++;
		}
		else if (tag == "f")
		{
			Tri tri;
			is >> tri.v[0] >> tri.v[1] >> tri.v[2];

			if (tri.v[0] < 0)
			{
				tri.v[0] += n + 1;
				tri.v[1] += n + 1;
				tri.v[2] += n + 1;
			}

			tri.v[0] --;
			tri.v[1] --;
			tri.v[2] --;
			tris_.push_back(tri);
		}
	}
	in.close();

	num_vtx_ = vtxs_.size();
	num_tri_ = tris_.size();

	// ����ģ�͵����ű�������֤ģ�͵Ĵ�С�ڴ����к��ʵ���ʾ
	scale_factor_ = fmax(fmax((x_max - x_min), (y_max - y_min)), (z_max - z_min));
	// ��ģ�͵��е�ƽ�Ƶ�ԭ��λ�õ�ƽ����
	translate_ = QVector3D(-(x_max + x_min) / 2.0f, -(y_max + y_min) / 2.0f, -(y_max + y_min) / 2.0f);
}

void TriMesh::setMesh()
{
	//  ������ɫ��
	bool success = shader_program_.addShaderFromSourceFile(QOpenGLShader::Vertex, "GLSL/vert.glsl");
	if (!success)
	{
		qDebug() << "shaderProgram addShaderFromSourceFile failed!" << shader_program_.log();
		return;
	}

	// Ƭ����ɫ��
	success = shader_program_.addShaderFromSourceFile(QOpenGLShader::Fragment, "GLSL/frag.glsl");
	if (!success)
	{
		qDebug() << "shaderProgram addShaderFromSourceFile failed!" << shader_program_.log();
		return;
	}

	// ������ɫ��
	success = shader_program_.link();
	if (!success)
	{
		qDebug() << "shaderProgram link failed!" << shader_program_.log();
		return;
	}

	vao_.create();
	vao_.bind();

	// ����������vbo����
	vbo_.create();
	vbo_.bind();
	vbo_.allocate(&vtxs_[0], vtxs_.size() * sizeof(Vec3f));

	// ����������ebo����
	ebo_.create();
	ebo_.bind();
	ebo_.allocate(&tris_[0], tris_.size() * sizeof(Tri));

	int attr = -1;
	attr = shader_program_.attributeLocation("aPos");
	shader_program_.setAttributeBuffer(attr, GL_FLOAT, 0, 3, sizeof(GLfloat) * 3);	// ���ý������ݵķ�ʽ
	shader_program_.enableAttributeArray(attr);

	vbo_.release();

	shader_program_.bind();
	// ��ͼview�任����
	QMatrix4x4 view;
	view.lookAt(QVector3D(0.0f, 0.0f, 4.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
	shader_program_.setUniformValue("view", view);

}

void TriMesh::drawMesh(QOpenGLFunctions_3_3_Core* ff, QMatrix4x4 &projection, QMatrix4x4 &model)
{
	model.translate(translate_);		// ƽ��ģ�͵�ԭ��
	model.scale(1.0f / scale_factor_ * 2.0f);		// ��ģ�ͽ���һ�����������ţ�ʹ��������
	shader_program_.bind();
	shader_program_.setUniformValue("projection", projection);
	shader_program_.setUniformValue("model", model);

	vao_.bind();

	ff->glDrawElements(GL_TRIANGLES, tris_.size() * 3, GL_UNSIGNED_INT, 0);		// ����

	shader_program_.release();
}

bool TriMesh::isEmpty()
{
	return num_vtx_ == 0;
}