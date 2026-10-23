package aspectratio

import (
	"testing"

	"github.com/MaaXYZ/MaaEnd/agent/go-service/pkg/pienv"
)

func TestIsCloudWebResolutionOK(t *testing.T) {
	cases := []struct {
		width  int32
		height int32
		want   bool
	}{
		{1280, 720, true},
		{1920, 1080, false},
		{1600, 900, false},
		{1280, 719, false},
		{0, 0, false},
	}
	for _, c := range cases {
		if got := isCloudWebResolutionOK(c.width, c.height); got != c.want {
			t.Fatalf("isCloudWebResolutionOK(%d, %d) = %v, want %v", c.width, c.height, got, c.want)
		}
	}
}

func TestDisplayControllerTypeCloudWeb(t *testing.T) {
	if got := displayControllerType(pienv.ControllerKindCloudWeb); got != "CloudWeb" {
		t.Fatalf("displayControllerType(cloudweb) = %q, want CloudWeb", got)
	}
}
