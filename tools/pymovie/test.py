import movie

m = movie.aeMovieData()

with open("ui/ui.aem", "rb") as f:
    if m.load(f) is False:
        print "failed"
        pass
    pass

#for composition in m.compositions:
#    print composition.name
#    pass

#for image in m.get_images():
#    print image
#    pass

print("======================================")
print("======================================")
print("======================================")

compositions = m.get_master_compositions()

for composition in compositions:
    print ("---------------------------------")
    print ("composition %s"%(composition))
    print ("---------------------------------")
    images = m.get_composition_images(composition)

    for image in images:
        print image
        pass
    pass