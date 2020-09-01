#pragma once
struct Edges_srv
{
	Texture2D areaTex;
	Texture2D searchTex;
	Texture2D colorTex;
};
struct Edges
{
	Edges_srv srv;
	Texture2D GetAreaTex() { return srv.areaTex; }
	Texture2D GetSearchTex() { return srv.searchTex; }
	Texture2D GetColorTex() { return srv.colorTex; }

};
 const Edges CreateEdges(Edges_srv srv)
{
	const Edges result = {srv
	};
	return result;
}
