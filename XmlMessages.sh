function get_files
{
    echo rkward/vnd.rkward.r.xml
}

function po_for_file
{
    case "$1" in
       rkward/vnd.rkward.r.xml)
           echo rkward_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       rkward/vnd.rkward.r.xml)
           echo comment
       ;;
    esac
}
