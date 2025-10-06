import { ImageCard, ImageItem } from '../components/ImageCard'

@Component
export struct DiscoverPage {
  @State imageList: ImageItem[] = [
    {
      id: '1',
      imageUrl: 'https://picsum.photos/200/300',
      title: '美丽的风景照片',
      userAvatar: 'https://picsum.photos/40/40',
      userId: 'user123',
      likeCount: 128,
      isLiked: false,
      imageHeight: 300
    },
    {
      id: '2',
      imageUrl: 'https://picsum.photos/200/250',
      title: '城市夜景',
      userAvatar: 'https://picsum.photos/40/40',
      userId: 'photographer',
      likeCount: 256,
      isLiked: true,
      imageHeight: 250
    },
    {
      id: '3',
      imageUrl: 'https://picsum.photos/200/350',
      title: '自然风光摄影作品',
      userAvatar: 'https://picsum.photos/40/40',
      userId: 'nature_lover',
      likeCount: 89,
      isLiked: false,
      imageHeight: 350
    },
    {
      id: '4',
      imageUrl: 'https://picsum.photos/200/280',
      title: '街头艺术',
      userAvatar: 'https://picsum.photos/40/40',
      userId: 'artist_joe',
      likeCount: 167,
      isLiked: false,
      imageHeight: 280
    },
    {
      id: '5',
      imageUrl: 'https://picsum.photos/200/320',
      title: '美食摄影',
      userAvatar: 'https://picsum.photos/40/40',
      userId: 'foodie',
      likeCount: 203,
      isLiked: true,
      imageHeight: 320
    },
    {
      id: '6',
      imageUrl: 'https://picsum.photos/200/290',
      title: '宠物写真',
      userAvatar: 'https://picsum.photos/40/40',
      userId: 'pet_lover',
      likeCount: 145,
      isLiked: false,
      imageHeight: 290
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
      Text('发现')
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


