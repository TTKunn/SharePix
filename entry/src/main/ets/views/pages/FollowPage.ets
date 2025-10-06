import { ImageCard, ImageItem } from '../components/ImageCard'

@Component
export struct FollowPage {
  @State imageList: ImageItem[] = [
    {
      id: '1',
      imageUrl: 'https://picsum.photos/200/280',
      title: '关注用户的精彩分享',
      userAvatar: 'https://picsum.photos/40/40',
      userId: 'friend_alice',
      likeCount: 95,
      isLiked: true,
      imageHeight: 280
    },
    {
      id: '2',
      imageUrl: 'https://picsum.photos/200/320',
      title: '朋友的旅行日记',
      userAvatar: 'https://picsum.photos/40/40',
      userId: 'traveler_bob',
      likeCount: 156,
      isLiked: false,
      imageHeight: 320
    },
    {
      id: '3',
      imageUrl: 'https://picsum.photos/200/260',
      title: '生活中的美好瞬间',
      userAvatar: 'https://picsum.photos/40/40',
      userId: 'life_lover',
      likeCount: 78,
      isLiked: true,
      imageHeight: 260
    },
    {
      id: '4',
      imageUrl: 'https://picsum.photos/200/340',
      title: '创意摄影作品',
      userAvatar: 'https://picsum.photos/40/40',
      userId: 'creative_mind',
      likeCount: 234,
      isLiked: false,
      imageHeight: 340
    }
  ]

  build() {
    Column() {
      this.buildHeader()
      this.buildContent()
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F5F5F5')
  }

  @Builder
  buildHeader() {
    Row() {
      Text('关注')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .fontColor('#333333')

      Blank()

      Button('搜索')
        .fontSize(14)
        .backgroundColor('#E0E0E0')
        .fontColor('#666666')
        .height(32)
        .borderRadius(16)
    }
    .width('100%')
    .height(56)
    .padding({ left: 16, right: 16 })
    .backgroundColor('#FFFFFF')
  }

  @Builder
  buildContent() {
    WaterFlow() {
      ForEach(this.imageList, (item: ImageItem, index: number) => {
        FlowItem() {
          ImageCard({
            item: item,
            onLikeClick: (id: string) => {
              this.handleLikeClick(id)
            }
          })
        }
      })
    }
    .columnsTemplate('1fr 1fr')
    .columnsGap(8)
    .rowsGap(8)
    .padding({ left: 8, right: 8, top: 8 })
    .width('100%')
    .layoutWeight(1)
  }

  handleLikeClick(id: string) {
    const index = this.imageList.findIndex(item => item.id === id)
    if (index !== -1) {
      this.imageList[index].isLiked = !this.imageList[index].isLiked
      this.imageList[index].likeCount += this.imageList[index].isLiked ? 1 : -1
    }
  }
}


