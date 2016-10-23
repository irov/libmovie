import movie

m = movie.aeMovieData()

f = open("ui/ui.aem", "rb")

with f:
    if m.load(f) is False:
        print "failed"
        pass
    pass

for composition in m.compositions:
    print composition.name
    pass