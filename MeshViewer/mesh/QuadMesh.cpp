#include "QuadMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <QOpenGLWidget>
#include <string>

QuadMesh::QuadMesh():vbo_(QOpenGLBuffer::VertexBuffer),
ebo_(QOpenGLBuffer::IndexBuffer)
{

}

QuadMesh::~QuadMesh()
{

}

void QuadMesh::loadFile(std::string file_name)
{
	std::ifstream in(file_name);

	if (!in)
	{
		std::cout << "The file can not be opened!" << std::endl;
		return;
	}

	REAL x_min = FLT_MAX, x_max = - FLT_MAX;
	REAL y_min = FLT_MAX, y_max = - FLT_MAX;
	REAL z_min = FLT_MAX, z_max = - FLT_MAX;

	int n = 0;	// 点的数量
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

			// 计算包围盒
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
			Quad quad;
			is >> quad.v[0] >> quad.v[1] >> quad.v[2] >> quad.v[3];

			if (quad.v[0] < 0)
			{
				quad.v[0] += n + 1;
				quad.v[1] += n + 1;
				quad.v[2] += n + 1;
				quad.v[3] += n + 1;
			}

			quad.v[0] --;
			quad.v[1] --;
			quad.v[2] --;
			quad.v[3] --;
			quads_.push_back(quad);
		}
	}
	in.close();

	num_vtx_ = vtxs_.size();
	num_quad_ = quads_.size();

	// 计算模型的缩放比例，保证模型的大小在窗口中合适的显示
	scale_factor_ = fmax(fmax((x_max - x_min), (y_max - y_min)), (z_max - z_min));
	// 将模型的中点平移到原点位置的平移量
	translate_ = QVector3D(-(x_max + x_min) / 2.0f, -(y_max + y_min) / 2.0f, -(y_max + y_min) / 2.0f);
}

void QuadMesh::setMesh()
{
	//  顶点着色器
	bool success = shader_program_.addShaderFromSourceFile(QOpenGLShader::Vertex, "GLSL/vert.glsl");
	if (!success)
	{
		qDebug() << "shaderProgram addShaderFromSourceFile failed!" << shader_program_.log();
		return;
	}

	// 片段着色器
	success = shader_program_.addShaderFromSourceFile(QOpenGLShader::Fragment, "GLSL/frag.glsl");
	if (!success)
	{
		qDebug() << "shaderProgram addShaderFromSourceFile failed!" << shader_program_.log();
		return;
	}

	// 链接着色器
	success = shader_program_.link();
	if (!success)
	{
		qDebug() << "shaderProgram link failed!" << shader_program_.log();
		return;
	}

	vao_.create();
	vao_.bind();

	// 创建并分配vbo对象
	vbo_.create();
	vbo_.bind();
	vbo_.allocate(&vtxs_[0], vtxs_.size() * sizeof(Vec3f));

	// 创建并分配ebo对象
	ebo_.create();
	ebo_.bind();
	ebo_.allocate(&quads_[0], quads_.size() * sizeof(Quad));

	int attr = -1;
	attr = shader_program_.attributeLocation("aPos");
	shader_program_.setAttributeBuffer(attr, GL_FLOAT, 0, 3, sizeof(GLfloat) * 3);	// 设置解析数据的方式
	shader_program_.enableAttributeArray(attr);

	vbo_.release();

	shader_program_.bind();
	// 视图view变换矩阵
	QMatrix4x4 view;
	view.lookAt(QVector3D(0.0f, 0.0f, 4.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
	shader_program_.setUniformValue("view", view);
}

void QuadMesh::drawMesh(QOpenGLFunctions_3_3_Core* ff, QMatrix4x4& projection, QMatrix4x4& model)
{
	model.translate(translate_);		// 平移模型到原点
	model.scale(1.0f / scale_factor_ * 2.0f);		// 对模型进行一定比例的缩放，使比例合适
	shader_program_.bind();
	shader_program_.setUniformValue("projection", projection);
	shader_program_.setUniformValue("model", model);

	//QOpenGLVertexArrayObject::Binder vaoBind(&vao_);
	vao_.bind();

	ff->glDrawElements(GL_QUADS, quads_.size() * 4, GL_UNSIGNED_INT, 0);		// 绘制

	shader_program_.release();
}

bool QuadMesh::isEmpty()
{
	return num_vtx_ == 0;
}

