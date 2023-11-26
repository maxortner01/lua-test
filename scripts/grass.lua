function GeneratePoints(mesh, mask)
    local size = mask:getSize()

    for y = 0, size.height - 1 do
        for x = 0, size.width - 1 do
            local color = mask:getPixel(x, y)

            local vertex = {
                position = { x = x, y = y },
                color = { r = color.r, g = 0, b = 0, a = color.a }
            }
            mesh:pushVertex(vertex)
        end
    end
end

function Start(world, entity)
    Debug.log("Generating grass mesh")

    local mesh = entity:getComponent(Component.CustomMesh)
    mesh:setPrimitiveType(PrimitiveType.Points)

    local mask = world:getResource(ResourceType.Image, "mask")
    GeneratePoints(mesh, mask)
end