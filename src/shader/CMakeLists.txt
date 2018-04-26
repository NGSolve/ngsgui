install(FILES
  mesh.frag mesh.vert mesh.geom solution.frag solution.geom solution.vert solution1d.frag solution1d.vert
  cross.vert cross.frag
  clipping.vert clipping.geom vector.geom isosurface.geom utils.inc interpolation.inc
  font.vert font.frag font.geom tess.tesc tess.tese lines.tesc lines.tese
  bbnd.vert bbnd.frag numbers.geom
  geo.vert filter_elements.vert filter_elements.geom
  colorbar.vert colorbar.frag
  DESTINATION ngsolve/gui/shader
)
