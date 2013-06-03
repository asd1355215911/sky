package skyd

import (
	"errors"
	"github.com/benbjohnson/go-raft"
	"net/http"
)

func (s *Server) addClusterHandlers() {
	s.ApiHandleFunc("/cluster", nil, s.getClusterHandler).Methods("GET")
	s.ApiHandleFunc("/cluster/commands", nil, s.doClusterCommandHandler).Methods("POST")
	s.ApiHandleFunc("/cluster/nodes", &CreateNodeCommand{}, s.createClusterNodeHandler).Methods("POST")
}

// GET /cluster
func (s *Server) getClusterHandler(w http.ResponseWriter, req *http.Request, params interface{}) (interface{}, error) {
	return s.cluster.serialize(), nil
}

// POST /cluster/commands
func (s *Server) doClusterCommandHandler(w http.ResponseWriter, req *http.Request, params interface{}) (interface{}, error) {
	command := params.(raft.Command)
	return nil, s.ExecuteClusterCommand(command)
}

// POST /cluster/nodes
func (s *Server) createClusterNodeHandler(w http.ResponseWriter, req *http.Request, params interface{}) (interface{}, error) {
	command := params.(*CreateNodeCommand)

	// Generate a node id if one is not passed in.
	if command.NodeId == "" {
		command.NodeId = NewNodeId()
	}

	// Retrieve a group to add to if one is not specified.
	if command.NodeGroupId == "" {
		group := s.cluster.GetAvailableNodeGroup()
		if group == nil {
			return nil, errors.New("No groups available")
		}
		command.NodeGroupId = group.id
	}

	return nil, s.ExecuteClusterCommand(command)
}