install(FILES
  mesh.frag mesh.vert solution.frag solution.vert solution1d.frag solution1d.vert
  cross.vert cross.frag
  clipping.vert clipping.geom vector.geom isosurface.geom elements.geom elements.frag elements.vert utils.inc interpolation.inc
  font.vert font.frag font.geom tess.tesc tess.tese
  bbnd.vert bbnd.frag
  geo.vert
  DESTINATION ngsolve/gui/shader
)
