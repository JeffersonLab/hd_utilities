# associative array Globus collection name -> Globus collection UUID
# source this to use it

declare -A GLOBUS_COLLECTIONS=(
  ["NERSC DTN jlab Collab"]="2f299200-6b58-4f81-bacd-b947873986db"  # for data inbound to and outbound from NERSC
  # ["NERSC Perlmutter jlab Collab"]="1933a329-8a7b-4bfa-996a-ddf25c7e97b5"
  # ["NERSC Perlmutter"]="6bdc7956-fc0f-4ad2-989c-7aa5ee643a79"
  ["JLAB#SWIF"]="086540a1-9598-445e-8637-ac2f3997874f"  # for data outbound from JLab
  ["jlab#gw1"]="b0fca1ad-f485-4a00-8fcd-bca0b93a2a1c"  # for data inbound to JLab
  # ["jlab#gw2"]="a2f9c453-2bb6-4336-919d-f195efcf327b"
  # ["jlab#scidtn1"]="81b20848-b468-11e9-98d4-0a63aa6b37da"
)
