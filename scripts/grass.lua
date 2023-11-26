function GeneratePoints(mesh, mask)
    local size = mask:getSize()

    math.randomseed(os.time())
    for y = 0, size.height - 1 do
        for x = 0, size.width - 1 do
            local color = mask:getPixel(x, y)

            local vertex = {
                position = { 
                    x = x + (math.random() * 2.0 - 1.0) * 0.9, 
                    y = y + (math.random() * 2.0 - 1.0) * 0.9 
                },
                color = { r = color.r, g = 0, b = 0, a = 255 }
            }
            
            if math.random() < color.a / 255.0 then
                mesh:pushVertex(vertex) 
            end
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

    -- When you push vertex it should go to RAM, a std::vector
    -- but doing syncPoints will allocate and copy over the data into the GPU
    -- or vice versa
    -- Only relevant when OpenGL backend is written
    -- flushRAM() should clear the RAM memory

    -- mesh:syncPoints() 
    -- mesh:flushRAM()