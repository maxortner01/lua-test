function Start(world, entity)
    Debug.log("Generating grass mesh")

    local mesh = entity:getComponent(Component.CustomMesh)
    mesh:setPrimitiveType(PrimitiveType.Triangles)
    
    local vertex = {
        position = { x = 0, y = 0 },
        color = { r = 255, g = 0, b = 0, a = 255 }
    }

    mesh:pushVertex(vertex)

    vertex.position.x = 100
    mesh:pushVertex(vertex)

    vertex.position.x = 0
    vertex.position.y = 100
    mesh:pushVertex(vertex)
end