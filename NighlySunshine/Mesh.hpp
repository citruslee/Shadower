#pragma once

#include "tiny_obj_loader.h"
struct VERTEX
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

class Mesh
{
public:
	~Mesh()
	{
		pVBuffer->Release();
		pIBuffer->Release();
	}

	void DrawMesh(ID3D11DeviceContext *devcon)
	{
		UINT stride = sizeof(VERTEX);
		UINT offset = 0;
		if (isIndexed)
		{
			devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);
			devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);

			devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			devcon->DrawIndexed(indices.size(), 0, 0);
		}
		else
		{
			devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);

			devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			devcon->Draw(vertices.size(), 0);
		}
	}

	void LoadObj(ID3D11Device *dev, ID3D11DeviceContext *devcon, const char *filename)
	{
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string errors;

		auto m_valid = tinyobj::LoadObj(shapes, materials, errors, filename);
		assert(m_valid);
		for (const auto& shape : shapes)
		{
			auto firstVertex = vertices.size();

			const auto& mesh = shape.mesh;
			const auto vertexCount = mesh.positions.size() / 3;
			const bool haveNormals = mesh.positions.size() == mesh.normals.size();
			const bool haveTexCoords = mesh.positions.size() == mesh.texcoords.size();
			for (auto i = 0; i < vertexCount; ++i)
			{
				VERTEX v = {};
				v.pos.x = mesh.positions[i * 3 + 0];
				v.pos.z = mesh.positions[i * 3 + 1];
				v.pos.y = mesh.positions[i * 3 + 2];
				v.color = DirectX::XMFLOAT4(0.5, 0.5, 0.5, 1);
				vertices.push_back(v);
			}

			for (auto &index : mesh.indices)
			{
				indices.push_back(index + firstVertex);
			}
		}

		D3D11_BUFFER_DESC bd = D3D11_BUFFER_DESC();
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(VERTEX) * vertices.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_BUFFER_DESC indexBufferDesc = D3D11_BUFFER_DESC();
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned int) * indices.size();
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		dev->CreateBuffer(&bd, NULL, &pVBuffer);

		D3D11_MAPPED_SUBRESOURCE ms;
		devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, vertices.data(), vertices.size() * sizeof(VERTEX));
		devcon->Unmap(pVBuffer, NULL);

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = indices.data();
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		dev->CreateBuffer(&indexBufferDesc, &InitData, &pIBuffer);
		isIndexed = true;
	}

	std::vector<VERTEX> &GetVertices()
	{
		return vertices;
	}

	std::vector<DirectX::XMFLOAT3> GetVertexPositions()
	{
		std::vector<DirectX::XMFLOAT3> pvec;
		for (auto &v : vertices)
		{
			pvec.push_back(v.pos);
		}
		return pvec;
	}

	std::vector<int> GetTriangles()
	{
		std::vector<int> pvec;
		for (auto &i : indices)
		{
			pvec.push_back(i);
		}
		return pvec;
	}

private:
	std::vector<VERTEX> vertices;
	std::vector<int> indices;
	bool isIndexed = false;

	ID3D11Buffer *pVBuffer = nullptr;
	ID3D11Buffer *pIBuffer = nullptr;
};
